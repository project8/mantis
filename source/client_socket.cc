#include "client_socket.hh"

#include "exception.hh"

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

    client_socket::client_socket() :
            f_connection( NULL )
    {
    }

    client_socket::~client_socket()
    {
    }

    void client_socket::open( const std::string& a_host, const int& a_port )
    {
        cout << "opening client socket with host <" << a_host << "> on port <" << a_port << ">" << endl;

        //find host
        hostent* t_host = gethostbyname( a_host.c_str() );
        if( t_host == NULL )
        {
            throw exception() << "could not find host <" << a_host << ">";
            return;
        }

        cout << "host found..." << endl;

        //prepare address structure
        socklen_t t_socket_length = sizeof( sockaddr_in );
        sockaddr_in* t_address = new sockaddr_in();
        ::memset( t_address, 0, t_socket_length );
        t_address->sin_family = AF_INET;
        ::memcpy( t_host->h_addr_list[0], &(t_address->sin_addr.s_addr), t_host->h_length );
        t_address->sin_port = htons( a_port );

        cout << "client address prepared..." << endl;

        //open socket
        int t_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if( t_socket < 0 )
        {
            throw exception() << "could not create client socket";
            return;
        }

        cout << "client socket opened..." << endl;

        //connect socket
        if( ::connect( t_socket, (sockaddr*) (t_address), sizeof( sockaddr_in ) ) < 0 )
        {
            throw exception() << "could not create connection";
        }
        f_connection = new connection( t_socket, t_address );

        cout << "client socket connected..." << endl;

        return;
    }

    void client_socket::close()
    {
        //delete connection
        delete f_connection;
    }

    connection* client_socket::get_connection()
    {
        //return connection
        return f_connection;
    }

}
