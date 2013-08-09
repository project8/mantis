#include "parser.hh"
#include "client.hh"
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

    client* t_client = new client( t_parser.get_required< string >( "host" ), t_parser.get_required< int >( "port" ) );

    string t_message;

    t_message.assign( "porkslaps mc scouts" );

    t_client->write( t_message );

    cout << "[test_mantis_client] sent message <" << t_message << ">" << endl;

    t_client->read( t_message );

    cout << "[test_mantis_client] received message <" << t_message << ">" << endl;

    delete t_client;

    return 0;
}


