#include "mt_client.hh"

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

    client::client( const std::string& a_host, const int& a_port ) :
            connection( -1, NULL )
    {
        //cout << "[client] creating client with host <" << a_host << "> on port <" << a_port << ">" << endl;

        //find host
        hostent* t_host = gethostbyname( a_host.c_str() );
        if( t_host == NULL )
        {
            throw exception() << "[client] could not find host <" << a_host << ">\n";
            return;
        }

        //cout << "[client] host found..." << endl;

        //initialize address
        socklen_t t_address_length = sizeof(sockaddr_in);
        f_address = new sockaddr_in();
        ::memset( f_address, 0, t_address_length );

        //prepare address
        f_address->sin_family = AF_INET;
        ::memcpy( t_host->h_addr_list[ 0 ], &(f_address->sin_addr.s_addr), t_host->h_length );
        f_address->sin_port = htons( a_port );

        //cout << "[client] address prepared..." << endl;

        //open socket
        f_socket = ::socket( AF_INET, SOCK_SEQPACKET, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[client] could not create socket\n";
            return;
        }

        //cout << "[client] socket opened..." << endl;

        //connect socket
        if( ::connect( f_socket, (sockaddr*) (f_address), t_address_length ) < 0 )
        {
            throw exception() << "could not create connection\n";
        }

        //cout << "[client] socket connected..." << endl;

        return;
    }

    client::~client()
    {
    }

}
