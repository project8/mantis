#include "parser.hh"
#include "client.hh"
#include "context.hh"
#include "time.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int analyze_status( context* t_context )
{
    switch( t_context->get_status()->state() )
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
    }
}

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    client* t_client = new client( t_parser.get_required< string >( "host" ), t_parser.get_required< int >( "port" ) );
    context* t_context = new context();
    t_context->set_connection( t_client );

    t_context->get_request()->set_file( "/data/ohgod.egg" );
    t_context->get_request()->set_description( "junk" );
    t_context->get_request()->set_date( get_string_time() );
    t_context->get_request()->set_mode( request_mode_t_single );
    t_context->get_request()->set_rate( 800.0 );
    t_context->get_request()->set_duration( 2000.0 );

    cout << "[test_mantis_client] sending request..." << endl;
    cout << t_context->get_request()->DebugString() << endl;

    t_context->push_request();

    t_context->pull_status();
    analyze_status( t_context );

//    do
//    {
//        t_context->pull_status();
//    }
//    while( analyze_status( t_context ) == 0 );

    cout << "[test_mantis_client] done" << endl;

    delete t_context;
    delete t_client;

    return 0;
}

