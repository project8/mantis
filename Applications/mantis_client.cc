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
#include "mt_client_file_writing.hh"
#include "mt_client_worker.hh"
#include "mt_client.hh"
#include "mt_exception.hh"
#include "mt_file_writer.hh"
#include "mt_run_context_dist.hh"
#include "mt_thread.hh"
#include "thorax.hh"
using namespace mantis;

#include <algorithm> // for min
#include <string>
#include <unistd.h>
using std::string;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

#define RETURN_SUCCESS 1
#define RETURN_ERROR -1
#define RETURN_CANCELED -2
#define RETURN_REVOKED -3

int main( int argc, char** argv )
{
    client_config t_cc;
    configurator t_config( argc, argv, &t_cc );

    cout << "[mantis_client] creating request objects..." << endl;

    bool t_client_writes_file = true;
    if( t_config.get_string_required( "file-writer" ) == std::string( "server" ) )
    {
        t_client_writes_file = false;
    }

    string t_request_host = t_config.get_string_required( "host" );
    int t_request_port = t_config.get_int_required( "port" );

    string t_write_host;
    int t_write_port = -1;
    if( t_client_writes_file )
    {
        t_write_host = t_config.get_string_required( "client-host" );
        t_write_port = t_config.get_int_optional( "client-port", t_request_port + 1 );
    }

    double t_duration = t_config.get_double_required( "duration" );
    useconds_t t_wait_during_run = std::min( ( useconds_t )1000, ( useconds_t )( t_duration / 10. ) );

    run_context_dist t_run_context;

    request* t_request = t_run_context.lock_request_out();
    t_request->set_write_host( t_write_host );
    t_request->set_write_port( t_write_port );
    t_request->set_file( t_config.get_string_required( "file" ) );
    t_request->set_description( t_config.get_string_optional( "description", "default client run" ) );
    t_request->set_date( get_absolute_time_string() );
    t_request->set_mode( (request_mode_t)t_config.get_int_required( "mode" ) );
    t_request->set_rate( t_config.get_double_required( "rate" ) );
    t_request->set_duration( t_duration );
    t_request->set_file_write_mode( request_file_write_mode_t_local );
    if( t_client_writes_file )
    {
        t_request->set_file_write_mode( request_file_write_mode_t_remote );
    }
    t_run_context.unlock_outbound();

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
        return RETURN_ERROR;
    }

    t_run_context.set_connection( t_request_client );


    cout << "[mantis_client] starting communicator" << endl;

    thread t_comm_thread( &t_run_context );
    t_comm_thread.start();


    cout << "[mantis_client] sending request..." << endl;

    if( ! t_run_context.push_request() )
    {
        t_run_context.cancel();
        t_comm_thread.cancel();
        cerr << "[mantis_client] error sending request" << endl;
        return RETURN_ERROR;
    }

    useconds_t t_sleep_time = 100;  // in usec

    while( true )
    {
        usleep( t_sleep_time );

        //t_run_context->pull_status( MSG_WAITALL );
        status_state_t t_state = t_run_context.lock_status_in()->state();
        t_run_context.unlock_inbound();

        if( t_state == status_state_t_acknowledged )
        {
            cout << "[mantis_client] run request acknowledged...\n" << endl;
            break;
        }

        if( t_state == status_state_t_error )
        {
            cerr << "[mantis_client] error reported; run was not acknowledged\n" << endl;
            t_run_context.cancel();
            t_comm_thread.cancel();
            return RETURN_ERROR;
        }

        if( t_state == status_state_t_revoked )
        {
            cout << "[mantis_client] request revoked; run did not take place\n" << endl;
            t_run_context.cancel();
            t_comm_thread.cancel();
            return RETURN_ERROR;
        }

        if( t_state != status_state_t_created )
        {
            cerr << "[mantis_client] server reported unusual status: " << t_state << endl;
            t_run_context.cancel();
            t_comm_thread.cancel();
            return RETURN_ERROR;
        }
    }

    // Server is now waiting for a client status update


    /****************************************************************/
    /*********************** file writing ***************************/
    /****************************************************************/
    client_file_writing* t_file_writing = NULL;
    if( t_client_writes_file )
    {
        cout << "[mantis_client] creating file-writing objects..." << endl;

        try
        {
            t_file_writing = new client_file_writing( &t_run_context, t_write_port );
        }
        catch( exception& e )
        {
            cerr << "[mantis_client] error setting up file writing: " << e.what() << endl;
            t_run_context.cancel();
            t_comm_thread.cancel();
            return RETURN_ERROR;
        }

    }
    /****************************************************************/
    /****************************************************************/
    /****************************************************************/


    cout << "[mantis_client] transmitting status: ready" << endl;

    t_run_context.lock_client_status_out()->set_state( client_status_state_t_ready );

    bool t_push_result = t_run_context.push_client_status_no_mutex();
    t_run_context.unlock_outbound();
    if( ! t_push_result )
    {
        cerr << "[mantis_client] error sending client status" << endl;
        delete t_file_writing;
        t_run_context.cancel();
        t_comm_thread.cancel();
        return RETURN_ERROR;
    }

    // =0 -> in progress
    // <0 -> unsuccessful
    // >0 -> successful
    int t_run_success = 0;
    while( true )
    {
        usleep( t_sleep_time );

        //t_run_context->pull_status( MSG_WAITALL );
        status_state_t t_state = t_run_context.lock_status_in()->state();
        t_run_context.unlock_inbound();

        if( t_state == status_state_t_waiting )
        {
            cout << "[mantis_client] waiting for run...\n" << endl;
            t_sleep_time = 100;
            continue;
        }

        if( t_state == status_state_t_started )
        {
            cout << "[mantis_client] run has started...\n" << endl;
            t_sleep_time = t_wait_during_run;
            continue;
        }

        if( t_state == status_state_t_running )
        {
            cout << "[mantis_client] run is in progress...\n" << endl;
            t_sleep_time = t_wait_during_run;
            continue;
        }

        if( t_state == status_state_t_stopped )
        {
            cout << "[mantis_client] run status: stopped; data acquisition has finished\n" << endl;
            t_run_success = RETURN_SUCCESS;
            break;
        }

        if( t_state == status_state_t_error )
        {
            cout << "[mantis_client] error reported; run did not complete\n" << endl;
            t_run_success = RETURN_ERROR;
            break;
        }

        if( t_state == status_state_t_canceled )
        {
            cout << "[mantis_client] cancellation reported; some data may have been written\n" << endl;
            t_run_success = RETURN_CANCELED;
            break;
        }

        if( t_state == status_state_t_revoked )
        {
            cout << "[mantis_client] request revoked; run did not take place\n" << endl;
            t_run_success = RETURN_REVOKED;
            break;
        }

        if( t_client_writes_file && t_file_writing->is_done() )
        {
            cout << "[mantis_client] file writing is done, but run status still does not indicate run is complete" << endl;
            cout << "                exiting run now!" << endl;
            t_run_success = RETURN_CANCELED;
            break;
        }
    }



    /****************************************************************/
    /*********************** file writing ***************************/
    /****************************************************************/
    if( t_client_writes_file )
    {
        if( t_run_success < 0 )
        {
            t_file_writing->cancel();
        }

        cout << "[mantis_client] waiting for record reception to end..." << endl;

        t_file_writing->wait_for_finish();

        cout << "[mantis_client] shutting down record receiver" << endl;

        delete t_file_writing;
        t_file_writing = NULL;
    }
    /****************************************************************/
    /****************************************************************/
    /****************************************************************/



    if( t_run_success > 0 || t_run_success == RETURN_CANCELED )
    {
        cout << "[mantis_client] receiving response..." << endl;

        if( ! t_run_context.pull_response() )
        {
            cerr << "error receiving response" << endl;
            t_run_context.cancel();
            t_comm_thread.cancel();
            return RETURN_ERROR;
        }

        response* t_response = t_run_context.lock_response_in();
        cout << "[mantis_client] digitizer summary:\n";
        cout << "  record count: " << t_response->digitizer_records() << " [#]\n";
        cout << "  acquisition count: " << t_response->digitizer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_response->digitizer_live_time() << " [sec]\n";
        cout << "  dead time: " << t_response->digitizer_dead_time() << " [sec]\n";
        cout << "  megabytes: " << t_response->digitizer_megabytes() << " [Mb]\n";
        cout << "  rate: " << t_response->digitizer_rate() << " [Mb/sec]\n";

        cout << endl;

        cout << "[mantis_client] writer summary:\n";
        cout << "  record count: " << t_response->writer_records() << " [#]\n";
        cout << "  acquisition count: " << t_response->writer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_response->writer_live_time() << " [sec]\n";
        cout << "  megabytes: " << t_response->writer_megabytes() << "[Mb]\n";
        cout << "  rate: " << t_response->writer_rate() << " [Mb/sec]\n";

        cout << endl;

        t_run_context.unlock_inbound();
    }

    t_run_context.cancel();
    t_comm_thread.cancel();

    return t_run_success;
}
