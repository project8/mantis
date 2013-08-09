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

    connection::connection( const int& a_socket, const sockaddr_in* an_address ) :
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

    void connection::write( const std::string& a_message )
    {
        //zero out buffer
        ::memset( f_buffer_content, 0, f_buffer_length );

        //copy message into buffer
        ::memcpy( f_buffer_content, a_message.c_str(), a_message.length() );

        //write to socket
        int t_written_length = ::write( f_socket, f_buffer_content, a_message.length() );
        if( t_written_length < 0 )
        {
            throw exception() << "could not write to socket";
            return;
        }

        return;
    }

    void connection::read( std::string& a_message )
    {
        //zero out buffer
        ::memset( f_buffer_content, 0, f_buffer_length );

        //read from socket
        int t_read_length = ::read( f_socket, f_buffer_content, f_buffer_length );
        if( t_read_length < 0 )
        {
            throw exception() << "could not read from socket";
            return;
        }

        //copy buffer into message
        a_message.assign( f_buffer_content );

        return;
    }

}
