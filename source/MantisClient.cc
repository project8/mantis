#include "client_socket.hh"
using mantis::client_socket;
using mantis::connection;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    client_socket t_socket;
    t_socket.open( "localhost", 51385 );

    connection* t_connection = t_socket.get_connection();

    string t_message;

    t_message.assign( "porkslaps mc scouts" );
    t_connection->write( t_message );

    cout << "sent message <" << t_message << ">" << endl;

    t_connection->read( t_message );

    cout << "received message <" << t_message << ">" << endl;

    t_socket.close();

    return 0;
}
