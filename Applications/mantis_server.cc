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
 *   - record-size (integer; number of samples in a record)
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
#include "mt_run_queue.hh"
#include "mt_server.hh"
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
    configurator* t_config = NULL;
    try
    {
        t_config = new configurator( argc, argv, &t_sc );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "unable to configure server: " << e.what() );
        return -1;
    }

    MTINFO( mtlog, "creating objects..." );

    size_t t_buffer_size, t_record_size, t_data_chunk_size;
    try
    {
        t_buffer_size = t_config->get< int >( "buffer-size" );
        t_record_size = t_config->get< int >( "record-size" );
        t_data_chunk_size = t_config->get< int >( "data-chunk-size" );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "required parameters were not available: " << e.what() );
        return -1;
    }

    // set up the server and request receiver

    server* t_server;
    try
    {
        t_server = new server( t_config->get< int >( "port" ) );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "unable to start server: " << e.what() );
        return -1;
    }

    condition t_queue_condition;
    run_queue t_run_queue;

    request_receiver t_receiver( t_server, &t_run_queue, &t_queue_condition );
    t_receiver.set_buffer_size( t_buffer_size );
    t_receiver.set_record_size( t_record_size );
    t_receiver.set_data_chunk_size( t_data_chunk_size );

    // set up the digitizer

    condition t_buffer_condition;
    buffer t_buffer( t_buffer_size, t_record_size );

    factory< digitizer >* t_dig_factory = NULL;
    digitizer* t_digitizer = NULL;
    try
    {
        t_dig_factory = factory< digitizer >::get_instance();
        t_digitizer = t_dig_factory->create( t_config->get< string >( "digitizer" ) );
        if( t_digitizer == NULL )
        {
            MTERROR( mtlog, "could not create digitizer <" << t_config->get< string >( "digitizer" ) << ">; aborting" );
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
    t_config->config()->add( "data-type-size", new param_value( t_digitizer->params().data_type_size ) );
    t_config->config()->add( "bit-depth", new param_value( t_digitizer->params().bit_depth ) );
    t_config->config()->add( "voltage-min", new param_value( t_digitizer->params().v_min ) );
    t_config->config()->add( "voltage-range", new param_value( t_digitizer->params().v_range ) );

    if(! t_digitizer->allocate( &t_buffer, &t_buffer_condition ) )
    {
        MTERROR( mtlog, "digitizer was not able to allocate the buffer" );
        delete t_server;
        return -1;
    }

    server_worker t_worker( t_config, t_digitizer, &t_buffer, &t_run_queue, &t_queue_condition, &t_buffer_condition );

    MTINFO( mtlog, "starting threads..." );

    try
    {
        thread t_queue_thread( &t_run_queue );
        thread t_receiver_thread( &t_receiver );
        thread t_worker_thread( &t_worker );

        signal_handler t_sig_hand;
        t_sig_hand.push_thread( &t_queue_thread );
        t_sig_hand.push_thread( &t_receiver_thread );
        t_sig_hand.push_thread( &t_worker_thread );

        t_queue_thread.start();
        t_receiver_thread.start();
        t_worker_thread.start();

        MTINFO( mtlog, "running..." );

        t_queue_thread.join();
        t_receiver_thread.join();
        t_worker_thread.join();

        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread(); // worker thread
            t_sig_hand.pop_thread(); // receiver thread
            t_sig_hand.pop_thread(); // queue thread
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

