#include "mt_server.hh"

#include "mt_exception.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    server::server( const int& a_port ) :
            f_socket( 0 ),
            f_address( NULL )
    {
        //cout << "[server] opening server socket on port <" << a_port << ">" << endl;

        //initialize address
        socklen_t t_socket_length = sizeof(sockaddr_in);
        f_address = new sockaddr_in();
        ::memset( f_address, 0, t_socket_length );

        //prepare address
        f_address->sin_family = AF_INET;
        f_address->sin_addr.s_addr = INADDR_ANY;
        f_address->sin_port = htons( a_port );

        //cout << "[server] address prepared..." << endl;

        //open socket
        f_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "could not create socket";
            return;
        }

        //cout << "[server] socket open..." << endl;

        //bind socket
        if( ::bind( f_socket, (const sockaddr*) (f_address), t_socket_length ) < 0 )
        {
            throw exception() << "[server] could not bind socket";
            return;
        }

        //cout << "[server] socket bound..." << endl;

        //start listening
        ::listen( f_socket, 10 );

        //cout << "[server] listening..." << endl;

        return;
    }

    server::~server()
    {
        //clean up server address
        delete f_address;

        //close server socket
        ::close( f_socket );
    }

    connection* server::get_connection()
    {
        int t_socket = 0;
        sockaddr_in* t_address = NULL;

        //initialize the new address
        socklen_t t_address_length = sizeof(sockaddr_in);
        t_address = new sockaddr_in();
        ::memset( t_address, 0, t_address_length );

        //accept a connection
        //blocks the thread while waiting for an incoming connection
        t_socket = ::accept( f_socket, (sockaddr*) (t_address), &t_address_length );
        if( t_socket < 0 )
        {
            throw exception() << "could not accept connection";
        }

        //cout << "[server] connection accepted..." << endl;

        //return a new connection
        return new connection( t_socket, t_address );
    }

}
