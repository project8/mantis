/*
 * mantis_client.cc
 *
 *      Author: Dan Furse
 *
 *  Client (file-writing) component of the DAQ
 *
 *  Usage:
 *  $> mantis_client host=<some host name> port=<some port number> file=<some file name> description=<describe your run> mode=<one or two channel> rate=<sampling rate> duration=<sampling duration>
 *
 *  Arguments:
 *  - host        (string; required):  address of the Mantis server host
 *  - port        (integer; required): port number opened by the server
 *  - file        (string; required):  egg filename
 *  - description (string; optional):  describe the run
 *  - mode        (integer; required): '1' for single-channel; '2' for double-channel
 *  - rate        (float; required):   digitization rate in MHz
 *  - duration    (float; required):   length of the run in ms
 *
 */

#include "mt_broker.hh"
#include "mt_connection.hh"
#include "mt_constants.hh"
#include "mt_logger.hh"
#include "mt_client_config.hh"
#include "mt_configurator.hh"
#include "mt_run_client.hh"

using namespace mantis;


MTLOGGER( mtlog, "mantis_client" );


int main( int argc, char** argv )
{
    try
    {
        client_config t_cc;
        configurator t_configurator( argc, argv, &t_cc );

        MTINFO( mtlog, "Connecting to AMQP broker" );

        param_node* t_broker_node = &t_configurator.config().remove( "amqp" )->as_node();

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

        std::string t_exchange;
        try
        {
            t_exchange = t_broker_node->get_value( "exchange" );
            t_broker->get_connection().amqp()->DeclareExchange( t_exchange, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, true );
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Unable to declare exchange <" << t_exchange << ">; aborting.\n(" << e.what() << ")" );
            t_broker->disconnect();
            return RETURN_ERROR;
        }

        delete t_broker_node;


        // Run the client

        run_client the_client( t_configurator.config(), t_configurator.exe_name(), t_exchange );

        the_client.execute();

        t_broker->disconnect();

        return the_client.get_return();
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

    return RETURN_ERROR;
}
