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

    cout << "[test_mantis_server] getting connection..." << endl;

    connection* t_connection = t_server->get_connection();

    string t_message;

    t_connection->read( t_message );

    cout << "[test_mantis_server] received message <" << t_message << ">" << endl;

    t_message.assign( "is a long donger chibb" );
    t_connection->write( t_message );

    cout << "[test_mantis_server] sent message <" << t_message << ">" << endl;

    delete t_connection;
    delete t_server;

    return 0;
}
