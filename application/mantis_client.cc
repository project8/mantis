#include "parser.hh"
#include "client.hh"
#include "context.hh"
using namespace mantis;

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    cout << "[mantis_client] creating objects..." << endl;

    client* t_client = new client( t_parser.get_required< string >( "host" ), t_parser.get_required< int >( "port" ) )
    context* t_context = new context();
    t_context->set_connection( t_client );

    cout << "[mantis_client] initializing request..." << endl;

    t_context->get_request()->set_file( t_parser.get_required< string >( "file" ) );
    t_context->get_request()->set_description( t_parser.get_required< string >( "description" ) );
    t_context->get_request()->set_date( get_string_time() );
    t_context->get_request()->set_mode( (request_mode_t) (t_parser.get_required< unsigned int >( "mode" )) );
    t_context->get_request()->set_rate( t_parser.get_required< double >( "rate" ) );
    t_context->get_request()->set_duration( t_parser.get_required< double >( "duration" ) );

    cout << "[mantis_client] sending request..." << endl;

    t_context->push_request();

    while( true )
    {
        t_context->pull_status();

        if( t_context->get_status()->state() == status_state_t_acknowledged )
        {
            cout << "[mantis_client] request acknowledged..." << '\r';
            cout.flush();
            continue;
        }

        if( t_context->get_status()->state() == status_state_t_waiting )
        {
            cout << "[mantis_client] request waiting...     " << '\r';
            cout.flush();
            continue;
        }

        if( t_context->get_status()->state() == status_state_t_started )
        {
            cout << "[mantis_client] request started...     " << '\r';
            cout.flush();
            continue;
        }

        if( t_context->get_status()->state() == status_state_t_running )
        {
            cout << "[mantis_client] request running...     " << '\r';
            cout.flush();
            continue;
        }

        if( t_context->get_status()->state() == status_state_t_stopped )
        {
            cout << "[mantis_client] request stopped...     " << '\r';
            cout.flush();
            cout << '\n' << endl;
            break;
        }
    }

    t_context->pull_response();

    cout << "[mantis_client] done!" << endl;

    delete t_context;
    delete t_client;

    return 0;
}
