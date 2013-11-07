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

#include "mt_parser.hh"
#include "mt_client.hh"
#include "mt_run_context.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    cout << "[mantis_client] creating objects..." << endl;

    client* t_client = new client( t_parser.get_required< string >( "host" ), t_parser.get_required< int >( "port" ) );
    run_context* t_run_context = new run_context();
    t_run_context->set_connection( t_client );

    t_run_context->get_request()->set_file( t_parser.get_required< string >( "file" ) );
    t_run_context->get_request()->set_description( t_parser.get_optional< string >( "description", "default mantis client description" ) );
    t_run_context->get_request()->set_date( get_absolute_time_string() );
    t_run_context->get_request()->set_mode( request_mode_t_single );
    t_run_context->get_request()->set_rate( t_parser.get_required< double >( "rate" ) );
    t_run_context->get_request()->set_duration( t_parser.get_required< double >( "duration" ) );

    cout << "[mantis_client] sending request..." << endl;

    if( ! t_run_context->push_request() )
    {
        cerr << "[mantis_client] error sending request" << endl;
        delete t_run_context;
        delete t_client;
        return -1;
    }

    while( true )
    {
        t_run_context->pull_status();
      //if( ! t_run_context->pull_status() )
      //{
      //    cerr << "[mantis_client] error pulling status; quitting" << endl;
      //    break;
      //}
        cout << "waiting for status" << endl;

        if( t_run_context->get_status()->state() == status_state_t_acknowledged )
        {
            cout << "[mantis_client] request acknowledged...";
            cout.flush();
            cout << "\n";
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_waiting )
        {
            cout << "[mantis_client] request waiting...     ";
            cout.flush();
            cout << "\n";
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_started )
        {
            cout << "[mantis_client] request started...     ";
            cout.flush();
            cout << "\n";
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_running )
        {
            cout << "[mantis_client] request running...     ";
            cout.flush();
            cout << "\n";
            continue;
        }

        if( t_run_context->get_status()->state() == status_state_t_stopped )
        {
            cout << "[mantis_client] request stopped...     ";
            cout.flush();
            cout << "\n";
            cout << endl;
            break;
        }

        if( t_run_context->get_status()->state() == status_state_t_error )
        {
            cout << "[mantis_client] request error...       ";
            cout.flush();
            cout << "\n";
            cout << endl;
            break;
        }
    }

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

    delete t_run_context;
    delete t_client;

    return 0;
}
