#include "client.hh"

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

    client::client( const std::string& a_host, const int& a_port ) :
            f_socket( -1 ),
            f_address( NULL )
    {
        cout << "[client] opening client socket with host <" << a_host << "> on port <" << a_port << ">" << endl;

        //find host
        hostent* t_host = gethostbyname( a_host.c_str() );
        if( t_host == NULL )
        {
            throw exception() << "[client] could not find host <" << a_host << ">";
            return;
        }

        cout << "[client] host found..." << endl;

        //prepare address structure
        socklen_t t_socket_length = sizeof(sockaddr_in);
        f_address = new sockaddr_in();
        ::memset( f_address, 0, t_socket_length );
        f_address->sin_family = AF_INET;
        ::memcpy( t_host->h_addr_list[ 0 ], &(f_address->sin_addr.s_addr), t_host->h_length );
        f_address->sin_port = htons( a_port );

        cout << "[client] address prepared..." << endl;

        //open socket
        f_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[client] could not create socket";
            return;
        }

        cout << "[client] socket opened..." << endl;

        //connect socket
        if( ::connect( f_socket, (sockaddr*) (f_socket), sizeof(sockaddr_in) ) < 0 )
        {
            throw exception() << "could not create connection";
        }

        cout << "[client] socket connected..." << endl;

        return;
    }

    client::~client()
    {
        ::close( f_socket );

        cout << "[client] socket closed..." << endl;
    }

    void client::write( const std::string& a_message )
    {
        //zero out buffer
        ::memset( f_buffer_content, 0, f_buffer_length );

        //copy message into buffer
        ::memcpy( f_buffer_content, a_message.c_str(), a_message.length() );

        //write to socket
        int t_written_length = ::write( f_socket, f_buffer_content, a_message.length() );
        if( t_written_length < 0 )
        {
            throw exception() << "[client] could not write to socket";
            return;
        }

        return;
    }

    void client::read( std::string& a_message )
    {
        //zero out buffer
        ::memset( f_buffer_content, 0, f_buffer_length );

        //read from socket
        int t_read_length = ::read( f_socket, f_buffer_content, f_buffer_length );
        if( t_read_length < 0 )
        {
            throw exception() << "[client] could not read from socket";
            return;
        }

        //copy buffer into message
        a_message.assign( f_buffer_content );

        return;
    }

}
