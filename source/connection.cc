#include "connection.hh"

#include "exception.hh"

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

    void connection::write( const char* a_message, size_t a_length )
    {
        int t_written_length = ::write( f_socket, a_message, a_length );
        if( t_written_length != a_length )
        {
            throw exception() << "could not write to socket";
            return;
        }

        return;
    }

    void connection::read( char* a_message, size_t a_length )
    {
        int t_read_length = ::read( f_socket, a_message, a_length );
        if( t_read_length < 0 )
        {
            throw exception() << "could not read from socket";
            return;
        }

        return;
    }

}
