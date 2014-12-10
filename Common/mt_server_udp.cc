#include "mt_server_udp.hh"

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
    MTLOGGER( mtlog, "server_udp" );

    int server_udp::f_last_errno = 0;

    server_udp::server_udp( const int& a_port ) :
            f_socket( 0 ),
            f_address( NULL ),
            f_client_address( NULL ),
            f_client_address_length( 0 )
    {
        //MTINFO( mtlog, "opening server_udp socket on port <" << a_port << ">" );

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
        f_socket = ::socket( AF_INET, SOCK_DGRAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[server_udp] could not create socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket open..." );

        //bind socket
        if( ::bind( f_socket, (const sockaddr*) (f_address), t_socket_length ) < 0 )
        {
            throw exception() << "[server_udp] could not bind socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket bound..." );

        //client address
        f_client_address = new sockaddr_in();
        f_client_address_length = sizeof( f_client_address );
        ::memset( f_client_address, 0, f_client_address_length );

        return;
    }

    server_udp::~server_udp()
    {
        //clean up server_udp address
        delete f_address;
        delete f_client_address;

        //close server_udp socket
        ::close( f_socket );
    }

    ssize_t server_udp::recvfrom( char* a_message, size_t a_size, int flags, int& ret_errno )
    {
        ssize_t t_recv_size = ::recvfrom( f_socket, a_message, a_size, flags, (sockaddr*)f_client_address, &f_client_address_length );

        if( t_recv_size > 0 )
        {
            return t_recv_size;
        }
        f_last_errno = errno;

        return t_recv_size;
    }

    sockaddr_in* server_udp::client_address()
    {
        return f_client_address;
    }
}
