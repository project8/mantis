/*
 * mantis_server.cc
 *
 *      Author: Dan Furse
 *
 *  Server component of the DAQ for the any of the available digitizers.
 *  Default configuration options are given in class server_config (see mt_server_config.cc)
 *
 *  The server can be configured in three ways, each of which can override previous values:
 *   1. Default configuration (see class server_config in mt_server_config.cc)
 *   2. Configuration file supplied by the user (json-formatted)
 *   3. Command line options
 *
 *  The server requires the following configuration values:
 *   - port (integer; must match the port used by the client)
 *   - buffer-size (integer; number of record blocks in the DMA buffer)
 *   - record-size (integer; number of samples in a block **** NOTE: this sets the number of samples in a block, even though it's called "record-size" ****)
 *
 *  Usage:
 *  $> mantis_server config=config_file.json [further configuration]
 *
 *  Arguments:
 *  - config (string; optional): json-formatted configuration file
 *  - further configuration: override or add new values
 *    format: name/type=value
 *    e.g.:   port/i=8235
 */

#include "mt_broker.hh"
#include "mt_condition.hh"
#include "mt_config_manager.hh"
#include "mt_constants.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_request_receiver.hh"
#include "mt_acq_request_db.hh"
#include "mt_server_config.hh"
#include "mt_server_worker.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
using namespace mantis;

using std::string;

MTLOGGER( mtlog, "mantis_server" );

int main( int argc, char** argv )
{
    MTINFO( mtlog, "Welcome to Mantis\n\n" <<
            "\t\t _______  _______  _       __________________ _______ \n" <<
            "\t\t(       )(  ___  )( \\    /|\\__   __/\\__   __/(  ____ \\\n" <<
            "\t\t| () () || (   ) ||  \\  ( |   ) (      ) (   | (    \\/\n" <<
            "\t\t| || || || (___) ||   \\ | |   | |      | |   | (_____ \n" <<
            "\t\t| |(_)| ||  ___  || (\\ \\) |   | |      | |   (_____  )\n" <<
            "\t\t| |   | || (   ) || | \\   |   | |      | |         ) |\n" <<
            "\t\t| )   ( || )   ( || )  \\  |   | |   ___) (___/\\____) |\n" <<
            "\t\t|/     \\||/     \\||/    \\_)   )_(   \\_______/\\_______)\n\n");

    try
    {
        server_config t_sc;
        configurator t_configurator( argc, argv, &t_sc );

        MTINFO( mtlog, "Connecting to AMQP broker" );

        const param_node* t_broker_node = &t_configurator.config().at( "amqp" )->as_node();

        broker* t_broker = broker::get_instance();

        if( ! t_broker->is_connected() )
        {
            if(! t_broker->connect( t_broker_node->get_value( "broker" ),
                    t_broker_node->get_value< unsigned >( "broker-port" ) ) )
            {
                MTERROR( mtlog, "Cannot create connection to AMQP broker" );
                return RETURN_ERROR;
            }
        }
        else
        {
            if( t_broker->get_address() != t_broker_node->get_value( "broker" ) ||
                    t_broker->get_port() != t_broker_node->get_value< unsigned >( "broker-port" ) )
            {
                MTERROR( mtlog, "Already connected to a different AMQP broker: " << t_broker->get_address() << ":" << t_broker->get_port() );
                return RETURN_ERROR;
            }
        }

        MTINFO( mtlog, "Creating server objects" );

        // device manager
        device_manager t_dev_mgr;

        // configuration manager
        config_manager t_config_mgr( t_configurator.config(), &t_dev_mgr );

        // run database and queue condition
        condition t_queue_condition;
        acq_request_db t_acq_request_db( &t_config_mgr, &t_queue_condition, t_configurator.exe_name() );

        // request receiver
        request_receiver t_receiver( &t_config_mgr, &t_acq_request_db );

        // server worker
        server_worker t_worker( &t_dev_mgr, &t_acq_request_db, &t_queue_condition );

        MTINFO( mtlog, "starting threads..." );

        thread t_receiver_thread( &t_receiver );
        thread t_worker_thread( &t_worker );

        signal_handler t_sig_hand;
        t_sig_hand.push_thread( &t_receiver_thread );
        t_sig_hand.push_thread( &t_worker_thread );

        t_receiver_thread.start();
        t_worker_thread.start();

        MTINFO( mtlog, "running..." );

        t_receiver_thread.join();
        t_worker_thread.join();

        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread(); // worker thread
            t_sig_hand.pop_thread(); // receiver thread
        }

        MTINFO( mtlog, "shutting down..." );

        t_broker->disconnect();
    }
    catch( param_exception& e )
    {
        MTERROR( mtlog, "configuration error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "mantis error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( std::exception& e )
    {
        MTERROR( mtlog, "std::exception caught: " << e.what() );
        return RETURN_ERROR;
    }

    return 0;
}

