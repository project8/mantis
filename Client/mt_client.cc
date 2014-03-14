#include "mt_client.hh"

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
    MTLOGGER( mtlog, "client" );

    client::client( const std::string& a_host, const int& a_port ) :
            connection( -1, NULL )
    {
        //MTINFO( mtlog, "creating client with host <" << a_host << "> on port <" << a_port << ">" );

        //find host
        hostent* t_host = gethostbyname( a_host.c_str() );
        if( t_host == NULL )
        {
            throw exception() << "[client] could not find host <" << a_host << ">\n";
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
        f_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if( f_socket < 0 )
        {
            throw exception() << "[client] could not create socket:\n\t" << strerror( errno );
            return;
        }

        //MTINFO( mtlog, "socket opened..." );

        //connect socket
        if( ::connect( f_socket, (sockaddr*) (f_address), t_address_length ) < 0 )
        {
            throw exception() << "[client] could not create connection:\n\t" << strerror( errno );
        }

        //MTINFO( mtlog, "socket connected..." );

        return;
    }

    client::~client()
    {
    }

}
