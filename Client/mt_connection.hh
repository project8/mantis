#ifndef MT_CONNECTION_HH_
#define MT_CONNECTION_HH_

#include "mt_exception.hh"

#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <sys/types.h>

#include <iostream>

namespace mantis
{

    class connection
    {
        public:
            connection( int a_socket, sockaddr_in* an_address );
            virtual ~connection();

            ssize_t send( const char* a_message, size_t a_size, int flags = 0 );
            ssize_t recv( char* a_message, size_t a_size, int flags = 0 );

            template< typename T >
            ssize_t send_type( T a_value, int flags = 0 );

            //size_t recv_size( int flags = 0 );
            template< typename T >
            T recv_type( int flags = 0 );

        protected:
            int f_socket;
            sockaddr_in* f_address;
    };

    template< typename T >
    ssize_t connection::send_type( T a_value, int flags )
    {
        //std::cout << "send_type is sending value " << a_value << std::endl;
        ssize_t t_written_size = ::send( f_socket, (void*)&a_value, sizeof( T ), flags );
        if( t_written_size != sizeof( T ) )
        {
            throw exception() << "send_type is unable to write <" << a_value << ">\n";
            return t_written_size;
        }
        return t_written_size;
    }

    template< typename T >
    T connection::recv_type( int flags )
    {
        T t_value = T();
        errno = 0;
        ssize_t t_recv_size = ::recv( f_socket, (void*)&t_value, sizeof( T ), flags );
        if( errno != EWOULDBLOCK && errno != EAGAIN )
        {
            throw exception() << "recv_type is unable to receive; connection has been closed\n";
        }
        //cout << "receiving something of size " << t_size << "; size read: " << t_recv_size << endl;
        return t_value;
    }

}


#endif
