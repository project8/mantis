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

    ssize_t connection::send( const char* a_message, size_t a_size )
    {
        send( (char*)&a_size, sizeof( size_t ) );
        ssize_t t_written_size = ::send( f_socket, a_message, a_size, 0 );
        if( t_written_size != a_size )
        {
            throw exception() << "could not write to socket (" << strerror(errno) << ")";
            return t_written_size;
        }

        return t_written_size;
    }

    ssize_t connection::recv( char* a_message, size_t a_size )
    {
        ssize_t t_read_size = ::recv( f_socket, a_message, a_size, 0 );
        if( t_read_size < 0 )
        {
            throw exception() << "could not read from socket (" << strerror(errno) << ")";
            return t_read_size;
        }

        return t_read_size;
    }

    size_t connection::recv_size()
    {
        size_t t_size = 0;
        ssize_t t_recv_size = ::recv( f_socket, (void*)&t_size, sizeof( size_t ), 0 );
        return t_size;
    }


}
