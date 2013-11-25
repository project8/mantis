#include "mt_configurator.hh"
#include "mt_server.hh"
#include "mt_request_dist.hh"
#include "mt_connection.hh"
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
    configurator t_configurator( argc, argv );

    cout << "[test_mantis_server] starting server..." << endl;

    server* t_server = new server( t_configurator.get_int_required( "port" ) );
    request_dist* t_request_dist = new request_dist();

    cout << "[test_mantis_server] waiting for connection..." << endl;

    t_request_dist->set_connection( t_server->get_connection() );

    t_request_dist->pull_request();

    cout << "[test_mantis_server] received request <" << t_request_dist->get_request()->DebugString() << ">" << endl;

    t_request_dist->get_status()->set_state( status_state_t_acknowledged );
    t_request_dist->push_status();

    t_request_dist->get_status()->set_state( status_state_t_started );
    t_request_dist->push_status();

    t_request_dist->get_status()->set_state( status_state_t_stopped );
    t_request_dist->push_status();

    cout << "[test_mantis_server] done" << endl;

    delete t_request_dist->get_connection();
    delete t_request_dist;
    delete t_server;

    return 0;
}
