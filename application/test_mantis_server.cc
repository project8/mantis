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

    server* t_server = new server( t_parser.get_required< int >( "port" ) );

    connection* t_connection = t_server->get_connection();

    string t_message;

    t_connection->read( t_message );

    cout << "[test_mantis_client] received message <" << t_message << ">" << endl;

    t_message.assign( "is a long donger chibb" );
    t_connection->write( t_message );

    cout << "[test_mantis_client] sent message <" << t_message << ">" << endl;

    delete t_connection;
    delete t_server;

    return 0;
}
