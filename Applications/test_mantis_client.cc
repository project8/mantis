#include "mt_parser.hh"
#include "mt_client.hh"
#include "mt_request_dist.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int analyze_status( request_dist* t_request_dist )
{
    switch( t_request_dist->get_status()->state() )
    {
        case status_state_t_acknowledged :
            cout << "[test_mantis_client] acknowledged..." << '\r';
            cout.flush();
            return 0;

        case status_state_t_waiting :
            cout << "[test_mantis_client] waiting..." << '\r';
            cout.flush();
            return 0;

        case status_state_t_started :
            cout << "[test_mantis_client] started..." << '\r';
            cout.flush();
            return 0;

        case status_state_t_running :
            cout << "[test_mantis_client] running..." << '\r';
            cout.flush();
            return 0;

        case status_state_t_stopped :
            cout << "[test_mantis_client] stopped..." << '\r';
            cout.flush();
            cout << endl;
            return 1;

        case status_state_t_error :
            cout << "[test_mantis_client] error..." << '\r';
            cout.flush();
            cout << endl;
            return -1;
    }
}

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    client* t_client = new client( t_parser.get_required< string >( "host" ), t_parser.get_required< int >( "port" ) );
    request_dist* t_request_dist = new request_dist();
    t_request_dist->set_connection( t_client );

    t_request_dist->get_request()->set_write_host( "" );
    t_request_dist->get_request()->set_write_port( -1 );
    t_request_dist->get_request()->set_file( "/data/ohgod.egg" );
    t_request_dist->get_request()->set_description( "junk" );
    t_request_dist->get_request()->set_date( get_absolute_time_string() );
    t_request_dist->get_request()->set_mode( request_mode_t_single );
    t_request_dist->get_request()->set_rate( 800.0 );
    t_request_dist->get_request()->set_duration( 2000.0 );
    t_request_dist->get_request()->set_file_write_mode( request_file_write_mode_t_local );

    cout << "[test_mantis_client] sending request..." << endl;
    cout << t_request_dist->get_request()->DebugString() << endl;

    t_request_dist->push_request();

    t_request_dist->pull_status();
    analyze_status( t_request_dist );

//    do
//    {
//        t_request_dist->pull_status();
//    }
//    while( analyze_status( t_run_context ) == 0 );

    cout << "[test_mantis_client] done" << endl;

    delete t_request_dist;
    delete t_client;

    return 0;
}

