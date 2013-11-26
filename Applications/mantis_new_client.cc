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

#include "mt_configurator.hh"
#include "mt_client_config.hh"
#include "mt_client_worker.hh"
#include "mt_client.hh"
#include "mt_exception.hh"
#include "mt_file_writer.hh"
#include "mt_request_dist.hh"
#include "mt_thread.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
#include <unistd.h>
using std::string;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

int main( int argc, char** argv )
{
    client_config t_cc;
    configurator t_config( argc, argv, &t_cc );

    cout << "[mantis_client] creating request objects..." << endl;

    string t_request_host = t_config.get_string_required( "host" );
    int t_request_port = t_config.get_int_required( "port" );

    string t_write_host = t_config.get_string_required( "client-host" );
    int t_write_port = t_config.get_int_optional( "client-port", t_request_port + 1 );

    double t_duration = t_config.get_double_required( "duration" );
    useconds_t t_wait_during_run = ( useconds_t )( t_duration / 10. );

    request_dist* t_connection_to_server = new request_dist();
    t_connection_to_server->get_request()->set_write_host( t_write_host );
    t_connection_to_server->get_request()->set_write_port( t_write_port );
    t_connection_to_server->get_request()->set_file( t_config.get_string_required( "file" ) );
    t_connection_to_server->get_request()->set_description( t_config.get_string_optional( "description", "default client run" ) );
    t_connection_to_server->get_request()->set_date( get_absolute_time_string() );
    t_connection_to_server->get_request()->set_mode( (request_mode_t)t_config.get_int_required( "mode" ) );
    t_connection_to_server->get_request()->set_rate( t_config.get_double_required( "rate" ) );
    t_connection_to_server->get_request()->set_duration( t_duration );

    // start the client for sending the request
    cout << "[mantis_client] connecting with the server..." << endl;

    client* t_request_client;
    try
    {
        t_request_client = new client( t_request_host, t_request_port );
    }
    catch( exception& e )
    {
        cerr << "[mantis_client] unable to start client: " << e.what() << endl;
        return -1;
    }

    t_connection_to_server->set_connection( t_request_client );


    cout << "[mantis_client] sending request..." << endl;

    if( ! t_connection_to_server->push_request() )
    {
        delete t_connection_to_server;
        delete t_request_client;
        cerr << "[mantis_client] error sending request" << endl;
        return -1;
    }

    useconds_t t_sleep_time = 100;  // in usec

    while( true )
    {
        usleep( t_sleep_time );

        t_connection_to_server->pull_status( MSG_WAITALL );

        if( t_connection_to_server->get_status()->state() == status_state_t_acknowledged )
        {
            cout << "[mantis_client] run request acknowledged...\n" << endl;
            break;
        }

        if( t_connection_to_server->get_status()->state() == status_state_t_error )
        {
            delete t_connection_to_server;
            delete t_request_client;
            cerr << "[mantis_client] error reported; run was not acknowledged\n" << endl;
            return -1;
        }

        cerr << "[mantis_client] server reported unusual status: " << t_connection_to_server->get_status()->state() << endl;
    }

    // Server is now waiting for a client status update

    /****************************************************************/
    /************************ move this *****************************/
    /****************************************************************/
    cout << "[mantis_client] creating run objects..." << endl;

    // get buffer size and record size from the request
    size_t t_buffer_size = t_connection_to_server->get_status()->buffer_size();
    size_t t_record_size = t_connection_to_server->get_status()->record_size();
    size_t t_data_chunk_size = t_connection_to_server->get_status()->data_chunk_size();

    // objects for receiving and writing data
    server* t_server;
    try
    {
        t_server = new server( t_write_port );
    }
    catch( exception& e)
    {
        cerr << "[mantis_client] unable to create record-receiver server: " << e.what() << endl;
        t_connection_to_server->get_client_status()->set_state( client_status_state_t_error );
        t_connection_to_server->push_client_status();
        delete t_connection_to_server;
        delete t_request_client;
        return -1;
    }

    condition t_buffer_condition;
    buffer t_buffer( t_buffer_size, t_record_size );

    record_receiver t_receiver( t_server, &t_buffer, &t_buffer_condition );
    t_receiver.set_data_chunk_size( t_data_chunk_size );

    file_writer t_writer;
    t_writer.set_buffer( &t_buffer, &t_buffer_condition );

    client_worker t_worker( t_connection_to_server->get_request(), &t_receiver, &t_writer, &t_buffer_condition );

    thread t_worker_thread( &t_worker );

    cout << "[mantis_client] starting record receiver" << endl;

    try
    {
        t_worker_thread.start();
    }
    catch( exception& e )
    {
        cerr << "[mantis_client] unable to start record-receiving server" << endl;
        delete t_connection_to_server;
        delete t_request_client;
        return -1;
    }
    /****************************************************************/
    /****************************************************************/
    /****************************************************************/


    cout << "[mantis_client] transmitting status: ready" << endl;

    t_connection_to_server->get_client_status()->set_state( client_status_state_t_ready );

    if( ! t_connection_to_server->push_client_status() )
    {
        delete t_connection_to_server;
        delete t_request_client;
        cerr << "[mantis_client] error sending client status" << endl;
        return -1;
    }

    // =0 -> in progress
    // <0 -> unsuccessful
    // >0 -> successful
    int t_run_success = 0;
    while( true )
    {
        usleep( t_sleep_time );

        t_connection_to_server->pull_status( MSG_WAITALL );

        if( t_connection_to_server->get_status()->state() == status_state_t_waiting )
        {
            cout << "[mantis_client] waiting for run...\n" << endl;
            t_sleep_time = 100;
            continue;
        }

        if( t_connection_to_server->get_status()->state() == status_state_t_started )
        {
            cout << "[mantis_client] run has started...\n" << endl;
            t_sleep_time = t_wait_during_run;
            continue;
        }

        if( t_connection_to_server->get_status()->state() == status_state_t_running )
        {
            cout << "[mantis_client] run is in progress...\n" << endl;
            t_sleep_time = t_wait_during_run;
            continue;
        }

        if( t_connection_to_server->get_status()->state() == status_state_t_stopped )
        {
            cout << "[mantis_client] run status: stopped; run should be complete\n" << endl;
            t_run_success = 1;
            break;
        }

        if( t_connection_to_server->get_status()->state() == status_state_t_error )
        {
            cout << "[mantis_client] error reported; run did not complete\n" << endl;
            t_run_success = -1;
            break;
        }
    }



    /****************************************************************/
    /************************ move this *****************************/
    /****************************************************************/
    cout << "[mantis_client] waiting for record reception to end..." << endl;

    t_worker_thread.join();

    delete t_server;

    cout << "[mantis_client] shutting down record receiver" << endl;
    /****************************************************************/
    /****************************************************************/
    /****************************************************************/



    if( t_run_success > 0 )
    {
        cout << "[mantis_client] receiving response..." << endl;

        if( ! t_connection_to_server->pull_response() )
        {
            cerr << "error receiving response" << endl;
            delete t_connection_to_server;
            delete t_request_client;
            return -1;
        }

        cout << "[mantis_client] digitizer summary:\n";
        cout << "  record count: " << t_connection_to_server->get_response()->digitizer_records() << " [#]\n";
        cout << "  acquisition count: " << t_connection_to_server->get_response()->digitizer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_connection_to_server->get_response()->digitizer_live_time() << " [sec]\n";
        cout << "  dead time: " << t_connection_to_server->get_response()->digitizer_dead_time() << " [sec]\n";
        cout << "  megabytes: " << t_connection_to_server->get_response()->digitizer_megabytes() << " [Mb]\n";
        cout << "  rate: " << t_connection_to_server->get_response()->digitizer_rate() << " [Mb/sec]\n";

        cout << endl;

        cout << "[mantis_client] writer summary:\n";
        cout << "  record count: " << t_connection_to_server->get_response()->writer_records() << " [#]\n";
        cout << "  acquisition count: " << t_connection_to_server->get_response()->writer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_connection_to_server->get_response()->writer_live_time() << " [sec]\n";
        cout << "  megabytes: " << t_connection_to_server->get_response()->writer_megabytes() << "[Mb]\n";
        cout << "  rate: " << t_connection_to_server->get_response()->writer_rate() << " [Mb/sec]\n";

        cout << endl;
    }

    delete t_connection_to_server;
    delete t_request_client;

    return t_run_success;
}
