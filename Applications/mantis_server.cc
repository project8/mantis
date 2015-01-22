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

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_request_receiver.hh"
#include "mt_run_database.hh"
#include "mt_server_tcp.hh"
#include "mt_server_config.hh"
#include "mt_server_worker.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
using namespace mantis;

using std::string;

MTLOGGER( mtlog, "mantis_server" );

int main( int argc, char** argv )
{
    MTDEBUG( mtlog, "Welcome to Mantis\n\n" <<
            "\t\t _______  _______  _       __________________ _______ \n" <<
            "\t\t(       )(  ___  )( \\    /|\\__   __/\\__   __/(  ____ \\\n" <<
            "\t\t| () () || (   ) ||  \\  ( |   ) (      ) (   | (    \\/\n" <<
            "\t\t| || || || (___) ||   \\ | |   | |      | |   | (_____ \n" <<
            "\t\t| |(_)| ||  ___  || (\\ \\) |   | |      | |   (_____  )\n" <<
            "\t\t| |   | || (   ) || | \\   |   | |      | |         ) |\n" <<
            "\t\t| )   ( || )   ( || )  \\  |   | |   ___) (___/\\____) |\n" <<
            "\t\t|/     \\||/     \\||/    \\_)   )_(   \\_______/\\_______)\n\n");

    server_config t_sc;
    configurator* t_configurator = NULL;
    try
    {
        t_configurator = new configurator( argc, argv, &t_sc );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "unable to configure server: " << e.what() );
        return -1;
    }

    param_node* t_config = t_configurator->config();

    MTINFO( mtlog, "creating objects..." );

    size_t t_buffer_size, t_block_size, t_data_chunk_size;
    try
    {
        t_buffer_size = t_config->get_value< int >( "buffer-size" );
        if( t_config->has( "record-size" ) )
        {
            MTWARN( mtlog, "The \"record-size\" option is deprecated; please use \"block-size\" instead" );
            t_block_size = t_config->get_value< int >( "record-size" );
        }
        else
        {
            t_block_size = t_config->get_value< int >( "block-size" );
        }
        t_data_chunk_size = t_config->get_value< int >( "data-chunk-size" );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "required parameters were not available: " << e.what() );
        return -1;
    }

    // set up the server and request receiver

    server_tcp* t_server;
    try
    {
        t_server = new server_tcp( t_config->get_value< int >( "port" ) );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "unable to start server: " << e.what() );
        return -1;
    }

    condition t_queue_condition;
    run_database t_run_database;

    request_receiver t_receiver( t_config, t_server, &t_run_database, &t_queue_condition, t_configurator->exe_name() );
    t_receiver.set_buffer_size( t_buffer_size );
    t_receiver.set_block_size( t_block_size );
    t_receiver.set_data_chunk_size( t_data_chunk_size );

    // set up the digitizer

    condition t_buffer_condition;
    buffer t_buffer( t_buffer_size, t_block_size );

    factory< digitizer >* t_dig_factory = NULL;
    digitizer* t_digitizer = NULL;
    try
    {
        t_dig_factory = factory< digitizer >::get_instance();
        t_digitizer = t_dig_factory->create( t_config->get_value< string >( "digitizer" ) );
        if( t_digitizer == NULL )
        {
            MTERROR( mtlog, "could not create digitizer <" << t_config->get_value< string >( "digitizer" ) << ">; aborting" );
            delete t_server;
            return -1;
        }
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "exception caught while creating digitizer: " << e.what() );
        delete t_server;
        return -1;
    }

    // get the digitizer parameters
    t_receiver.set_data_type_size( t_digitizer->params().data_type_size );
    t_receiver.set_bit_depth( t_digitizer->params().bit_depth );
    t_receiver.set_voltage_min( t_digitizer->params().v_min );
    t_receiver.set_voltage_range( t_digitizer->params().v_range );
    t_config->add( "data-type-size", new param_value( t_digitizer->params().data_type_size ) );
    t_config->add( "bit-depth", new param_value( t_digitizer->params().bit_depth ) );
    t_config->add( "voltage-min", new param_value( t_digitizer->params().v_min ) );
    t_config->add( "voltage-range", new param_value( t_digitizer->params().v_range ) );

    if(! t_digitizer->allocate( &t_buffer, &t_buffer_condition ) )
    {
        MTERROR( mtlog, "digitizer was not able to allocate the buffer" );
        delete t_server;
        return -1;
    }

    server_worker t_worker( t_config,
                            t_digitizer,
                            &t_buffer, &t_run_database,
                            &t_queue_condition, &t_buffer_condition,
                            t_configurator->exe_name() );

    MTINFO( mtlog, "starting threads..." );

    try
    {
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
    }
    catch( exception& e)
    {
        MTERROR( mtlog, "exception caught during server running: \n\t" << e.what() );
        return -1;
    }

    MTINFO( mtlog, "shutting down..." );

    delete t_server;
    delete t_config;

    return 0;
}

