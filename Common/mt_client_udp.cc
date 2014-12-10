#include "mt_client_udp.hh"

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
    MTLOGGER( mtlog, "client_udp" );

    int client_udp::f_last_errno = 0;

    client_udp::client_udp( const std::string& a_host, const int& a_port )
    {
        //MTINFO( mtlog, "creating client_udp with host <" << a_host << "> on port <" << a_port << ">" );

        //find host
        hostent* t_host = gethostbyname( a_host.c_str() );
        if( t_host == NULL )
        {
            throw exception() << "[client_udp] could not find host <" << a_host << ">\n";
            return;
        }

        //MTDEBUG( mtlog, "host found: " );

        //initialize address
        socklen_t t_address_length = sizeof(sockaddr_in);
        f_address = new sockaddr_in();
        ::memset( f_address, 0, t_address_length );

        //prepare address
        f_address->sin_family = AF_INET;
        ::memcpy( &(f_address->sin_addr.s_addr), t_host->h_addr_list[ 0 ], t_host->h_length );
        f_address->sin_port = htons( a_port );

        //MTINFO( mtlog, "address prepared..." );

        //open socket
        f_socket = ::socket( AF_INET, SOCK_DGRAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[client_udp] could not create socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket opened..." );

        return;
    }

    client_udp::~client_udp()
    {
    }

    ssize_t client_udp::send( const char* a_message, size_t a_size, int flags, int& ret_errno )
    {
        ssize_t t_written_size = ::send( f_socket, (void*)a_message, a_size, flags );

        if( t_written_size == (ssize_t)a_size )
        {
            return t_written_size;
        }
        f_last_errno = errno;

        return t_written_size;
    }
}
