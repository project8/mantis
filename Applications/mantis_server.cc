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
#include "mt_request_receiver.hh"
#include "mt_run_queue.hh"
#include "mt_server.hh"
#include "mt_server_config.hh"
#include "mt_server_worker.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
using namespace mantis;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    server_config t_sc;
    configurator t_config( argc, argv, &t_sc );

    cout << "[mantis_server] creating objects..." << endl;

    size_t t_buffer_size = t_config.get_int_required( "buffer-size" );
    size_t t_record_size = t_config.get_int_required( "record-size" );
    size_t t_data_chunk_size = t_config.get_int_required( "data-chunk-size" );

    // set up the server and request receiver

    server* t_server;
    try
    {
        t_server = new server( t_config.get_int_required( "port" ) );
    }
    catch( exception& e )
    {
        cerr << "[mantis_server] unable to start server: " << e.what();
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

    factory< digitizer >* t_dig_factory = factory< digitizer >::get_instance();
    digitizer* t_digitizer = t_dig_factory->create( t_config.get_string_required( "digitizer" ) );
    t_digitizer->allocate( &t_buffer, &t_buffer_condition );

    server_worker t_worker( &t_config, t_digitizer, &t_buffer, &t_run_queue, &t_queue_condition, &t_buffer_condition );

    cout << "[mantis_server] starting threads..." << endl;

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

        cout << "[mantis_server] running..." << endl;

        t_queue_thread.join();
        t_receiver_thread.join();
        t_worker_thread.join();
    }
    catch( exception& e)
    {
        cerr << "exception caught during server running: \n\t" << e.what() << endl;
        return -1;
    }

    cout << "[mantis_server] shutting down..." << endl;

    // ensure that all threads have cancelled
    sleep( 1 );

    delete t_server;

    return 0;
}

