#include "parser.hh"
#include "server.hh"
#include "connection.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    cout << "[test_mantis_server] starting server..." << endl;

    server* t_server = new server( t_parser.get_required< int >( "port" ) );
    context* t_context = new context();

    cout << "[test_mantis_server] waiting for connection..." << endl;

    t_context->set_connection( t_server->get_connection() );

    t_context->pull_request();

    cout << "[test_mantis_server] received request <" << t_context->get_request()->DebugString() << ">" << endl;

    t_context->get_status()->set_state( status_state_t_acknowledged );
    t_context->push_status();

    t_context->get_status()->set_state( status_state_t_started );
    t_context->push_status();

    t_context->get_status()->set_state( status_state_t_stopped );
    t_context->push_status();

    cout << "[test_mantis_server] done" << endl;

    delete t_context->get_connection();
    delete t_context;
    delete t_server;

    return 0;
}
