#include "mt_connection.hh"

#include "mt_exception.hh"

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

    connection::connection( int a_socket, sockaddr_in* an_address ) :
            f_socket( a_socket ),
            f_address( an_address )
    {
    }

    connection::~connection()
    {
        //clean up connection socket
        ::close( f_socket );

        //clean up connection address
        delete f_address;
    }

    ssize_t connection::write( const char* a_message, size_t a_length )
    {
        ssize_t t_written_length = ::write( f_socket, a_message, a_length );
        if( t_written_length != a_length )
        {
            throw exception() << "could not write to socket (" << strerror(errno) << ")";
            return t_written_length;
        }

        return t_written_length;
    }

    ssize_t connection::read( char* a_message, size_t a_length )
    {
        ssize_t t_read_length = ::read( f_socket, a_message, a_length );
        if( t_read_length < 0 )
        {
            throw exception() << "could not read from socket (" << strerror(errno) << ")";
            return t_read_length;
        }

        return t_read_length;
    }

}
