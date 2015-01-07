#include "mt_server_tcp.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace mantis
{
    MTLOGGER( mtlog, "server_tcp" );

    server_tcp::server_tcp( const int& a_port ) :
            f_socket( 0 ),
            f_address( NULL )
    {
        //MTINFO( mtlog, "opening server_tcp socket on port <" << a_port << ">" );

        //initialize address
        socklen_t t_socket_length = sizeof(sockaddr_in);
        f_address = new sockaddr_in();
        ::memset( f_address, 0, t_socket_length );

        //prepare address
        f_address->sin_family = AF_INET;
        f_address->sin_addr.s_addr = INADDR_ANY;
        f_address->sin_port = htons( a_port );

        //MTINFO( mtlog, "address prepared..." );

        //open socket
        f_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[server_tcp] could not create socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket open..." );

        //bind socket
        if( ::bind( f_socket, (const sockaddr*) (f_address), t_socket_length ) < 0 )
        {
            throw exception() << "[server_tcp] could not bind socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket bound..." );

        //start listening
        if( ::listen( f_socket, 10 ) < 0 )
        {
            throw exception() << "[server_tcp] listen failed:\n\t" << strerror( errno );
        }

        //MTINFO( mtlog, "listening..." );

        return;
    }

    server_tcp::~server_tcp()
    {
        //clean up server_tcp address
        delete f_address;

        //close server_tcp socket
        ::close( f_socket );
    }

    connection* server_tcp::get_connection()
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
            throw exception() << "[server_tcp] could not accept connection:\n\t" << strerror( errno );
        }

        //MTINFO( mtlog, "connection accepted..." );

        //return a new connection
        return new connection( t_socket, t_address );
    }

}
