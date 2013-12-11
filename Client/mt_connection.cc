#include "mt_connection.hh"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

//#include <iostream>
//using std::cout;
//using std::endl;

namespace mantis
{

    connection::connection( int a_socket, sockaddr_in* an_address ) :
            f_socket( a_socket ),
            f_address( an_address ),
            f_temp_errno( 0 )
    {
    }

    connection::~connection()
    {
        //clean up connection socket
        ::close( f_socket );

        //clean up connection address
        delete f_address;
    }

    ssize_t connection::send( const char* a_message, size_t a_size, int flags )
    {
        //cout << "sending message of size: " << a_size << endl;
        errno = 0;
        send_type( a_size, flags );
        ssize_t t_written_size = ::send( f_socket, (void*)a_message, a_size, flags );
        if( t_written_size == a_size )
        {
            return t_written_size;
        }
        f_temp_errno = errno;
        if( t_written_size < 0 )
        {
            if( f_temp_errno == EPIPE )
            {
                throw closed_connection() << "connection::send";
                return -1;
            }
            throw exception() << "send is unable to send message; error message: " << strerror( f_temp_errno );
            return -1;
        }
        throw exception() << "send did not send message correctly; write size was different than value size\n";
        return t_written_size;
    }

    ssize_t connection::recv( char* a_message, size_t a_size, int flags )
    {
        ssize_t t_recv_size = ::recv( f_socket, (void*)a_message, a_size, flags );
        if( t_recv_size > 0 )
        {
            return t_recv_size;
        }
        f_temp_errno = errno;
        if( t_recv_size == 0 && f_temp_errno != EWOULDBLOCK && f_temp_errno != EAGAIN )
        {
            throw closed_connection() << "connection::recv";
        }
        else if( t_recv_size < 0 )
        {
            throw exception() << "recv is unable to receive; error message: " << strerror( f_temp_errno ) << "\n";
        }
        // at this point t_recv_size must be 0, and errno is either EWOULDBLOCK or EAGAIN,
        // which means that there was no data available to receive, but nothing seems to be wrong with the connection
        return t_recv_size;
    }

    /*
    size_t connection::recv_size( int flags )
    {
        size_t t_size = 0;
        ssize_t t_recv_size = ::recv( f_socket, (void*)&t_size, sizeof( size_t ), flags );
        if( t_recv_size < 0 )
        {
            t_size = 0;
        }
        //cout << "receiving something of size " << t_size << "; size read: " << t_recv_size << endl;
        return t_size;
    }
    */

    closed_connection::closed_connection() :
            std::exception(),
            f_exception( "" )
    {
    }
    closed_connection::closed_connection( const closed_connection& an_exception ) :
            std::exception(),
            f_exception( an_exception.f_exception.str() )
    {
    }

    closed_connection::~closed_connection() throw ()
    {
    }

    const char* closed_connection::what() const throw ()
    {
        return f_exception.str().c_str();
    }


}
