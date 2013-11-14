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
#include "mt_file_writer.hh"
#include "mt_request_dist.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
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

    cout << "[mantis_client] creating objects..." << endl;

    string t_request_host = t_config.get_string_required( "host" );
    int t_request_port = t_config.get_int_required( "port" );

    string t_write_host = t_config.get_string_required( "client-host" );
    int t_write_port = t_request_port + 1;

    request_dist* t_request_dist = new request_dist();
    t_request_dist->get_request()->set_write_host( t_write_host );
    t_request_dist->get_request()->set_write_port( t_write_port );
    t_request_dist->get_request()->set_file( t_config.get_string_required( "file" ) );
    t_request_dist->get_request()->set_description( t_config.get_string_optional( "description", "default client run" ) );
    t_request_dist->get_request()->set_date( get_absolute_time_string() );
    t_request_dist->get_request()->set_mode( (request_mode_t)t_config.get_int_required( "mode" ) );
    t_request_dist->get_request()->set_rate( t_config.get_double_required( "rate" ) );
    t_request_dist->get_request()->set_duration( t_config.get_double_required( "duration" ) );

    // start the client for sending the request
    client* t_request_client = new client( t_request_host, t_request_port );
    t_request_dist->set_connection( t_request_client );

    // objects for receiving and writing data
    condition t_buffer_condition;
    buffer t_buffer( t_config.get_int_required( "buffer-size" ), t_config.get_int_required( "record-size" ) );

    file_writer t_writer( &t_buffer, &t_buffer_condition );

    client_worker t_worker( &t_writer, &t_buffer_condition );

    thread t_worker_thread( &t_worker );

    cout << "[mantis_client] starting run" << endl;

    try
    {
        t_worker_thread.start();

        cout << "[mantis_client] sending request..." << endl;

        if( ! t_request_dist->push_request() )
        {
            delete t_request_dist;
            delete t_request_client;
            throw exception() << "[mantis_client] error sending request";
        }

        cout << "[mantis_client] running..." << endl;

        t_worker_thread.join();
    }
    catch( exception& e)
    {
        cerr << "exception caught during client running" << e.what() << endl;
        return -1;
    }

    delete t_request_dist;
    delete t_request_client;




    // start server waiting for incoming write connection
    server* t_write_server = new server( t_write_port );



    // TODO: check with daq server on run status?
    //       eventually timeout?


    delete t_write_server;




    unsigned t_sleep_time = 1;

    // =0 -> in progress
    // <0 -> unsuccessful
    // >0 -> successful
    int t_run_success = 0;
    while( true )
    {
        // TODO: consider putting in a timeout for waiting for acknowledgement?
        sleep( t_sleep_time );

        t_run_context->pull_status();

        if( t_run_context->get_status()->state() == status_state_t_acknowledged )
        {
            cout << "[mantis_client] run request acknowledged...";
            cout.flush();
            cout << "\n";
            t_sleep_time = 1;
            break;
        }
    }



        if( t_run_context->get_status()->state() == status_state_t_waiting )
        {
            cout << "[mantis_client] waiting for run...";
            cout.flush();
            cout << "\n";
            t_sleep_time = 1;
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_started )
        {
            cout << "[mantis_client] run has started...";
            cout.flush();
            cout << "\n";
            t_sleep_time = 1;
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_running )
        {
            cout << "[mantis_client] run is in progress...";
            cout.flush();
            cout << "\n";
            t_sleep_time = 10;
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_stopped )
        {
            cout << "[mantis_client] run status: stopped; run should be complete";
            cout.flush();
            cout << "\n" << endl;
            t_run_success = 1;
            break;
        }

        if( t_run_context->get_status()->state() == status_state_t_error )
        {
            cout << "[mantis_client] error reported; run did not complete";
            cout.flush();
            cout << "\n" << endl;
            t_run_success = -1;
            break;
        }
    }

    if( t_run_success > 0 )
    {
        cout << "[mantis_client] receiving response..." << endl;

        if( ! t_run_context->pull_response() )
        {
            cerr << "error receiving response" << endl;
            delete t_run_context;
            delete t_client;
            return -1;
        }

        cout << "[mantis_client] digitizer summary:\n";
        cout << "  record count: " << t_run_context->get_response()->digitizer_records() << " [#]\n";
        cout << "  acquisition count: " << t_run_context->get_response()->digitizer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_run_context->get_response()->digitizer_live_time() << " [sec]\n";
        cout << "  dead time: " << t_run_context->get_response()->digitizer_dead_time() << " [sec]\n";
        cout << "  megabytes: " << t_run_context->get_response()->digitizer_megabytes() << " [Mb]\n";
        cout << "  rate: " << t_run_context->get_response()->digitizer_rate() << " [Mb/sec]\n";

        cout << endl;

        cout << "[mantis_client] writer summary:\n";
        cout << "  record count: " << t_run_context->get_response()->writer_records() << " [#]\n";
        cout << "  acquisition count: " << t_run_context->get_response()->writer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_run_context->get_response()->writer_live_time() << " [sec]\n";
        cout << "  megabytes: " << t_run_context->get_response()->writer_megabytes() << "[Mb]\n";
        cout << "  rate: " << t_run_context->get_response()->writer_rate() << " [Mb/sec]\n";
    }

    delete t_run_context;
    delete t_client;

    return t_run_success;
}
