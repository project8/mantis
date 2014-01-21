#include "mt_parser.hh"
#include "mt_client.hh"
#include "mt_run_context_dist.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int analyze_status( run_context_dist* t_run_context )
{
    status_state_t t_state = t_run_context->lock_status_in()->state();
    t_run_context->unlock_inbound();
    switch( t_state )
    {
        case status_state_t_created :
            cout << "[test_mantis_client] created..." << '\r';
            cout.flush();
            return 0;

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

        case status_state_t_canceled :
            cout << "[test_mantis_client] canceled..." << '\r';
            cout.flush();
            cout << endl;
            return -1;

        case status_state_t_revoked :
            cout << "[test_mantis_client] revoked..." << '\r';
            cout.flush();
            cout << endl;
            return -1;
    }
}

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    client* t_client = new client( t_parser.value_at( "host" )->get(), t_parser.value_at( "port" )->get< int >() );
    run_context_dist* t_run_context = new run_context_dist();
    t_run_context->set_connection( t_client );

    request* t_request = t_run_context->lock_request_out();
    t_request->set_write_host( "" );
    t_request->set_write_port( -1 );
    t_request->set_file( "/value/ohgod.egg" );
    t_request->set_description( "junk" );
    t_request->set_date( get_absolute_time_string() );
    t_request->set_mode( request_mode_t_single );
    t_request->set_rate( 800.0 );
    t_request->set_duration( 2000.0 );
    t_request->set_file_write_mode( request_file_write_mode_t_local );

    cout << "[test_mantis_client] sending request..." << endl;
    cout << t_request->DebugString() << endl;

    t_run_context->push_request_no_mutex();

    t_run_context->unlock_outbound();

    t_run_context->pull_status();
    analyze_status( t_run_context );

//    do
//    {
//        t_run_context->pull_status();
//    }
//    while( analyze_status( t_run_context ) == 0 );

    cout << "[test_mantis_client] done" << endl;

    delete t_run_context;
    delete t_client;

    return 0;
}

