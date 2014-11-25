#ifndef MT_CONNECTION_HH_
#define MT_CONNECTION_HH_

#include "mt_exception.hh"

#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace mantis
{
    enum socket_type
    {
        k_stream = SOCK_STREAM, // tcp
        k_dgram = SOCK_DGRAM,   // udp
        k_seqpacket = SOCK_SEQPACKET,
        k_raw = SOCK_RAW,
        k_rdm = SOCK_RDM
    };

    class closed_connection :
        public std::exception
    {
        public:
            closed_connection();
            closed_connection( const closed_connection& );
            ~closed_connection() throw ();

            template< class x_streamable >
            closed_connection& operator<<( const x_streamable& a_fragment )
            {
                f_exception << a_fragment;
                return *this;
            }

            virtual const char* what() const throw();

        private:
            std::stringstream f_exception;
    };


    class connection
    {
        public:
            connection( int a_socket, sockaddr_in* an_address );
            virtual ~connection();

            ssize_t send( const char* a_message, size_t a_size, int flags = 0, int& ret_errno = f_last_errno );
            ssize_t recv( char* a_message, size_t a_size, int flags = 0, int& ret_errno = f_last_errno );

            template< typename T >
            ssize_t send_type( T a_value, int flags = 0, int& ret_errno = f_last_errno );

            //size_t recv_size( int flags = 0 );
            template< typename T >
            T recv_type( int flags = 0, int& ret_errno = f_last_errno );

            bool set_send_timeout( unsigned sec, unsigned usec = 0, int& ret_errno = f_last_errno );
            bool set_recv_timeout( unsigned sec, unsigned usec = 0, int& ret_errno = f_last_errno );

            static int get_last_errno();

        protected:
            int f_socket;
            sockaddr_in* f_address;

        private:
            static int f_last_errno;
    };

    template< typename T >
    ssize_t connection::send_type( T a_value, int flags, int& ret_errno )
    {
        //std::cout << "send_type is sending value " << a_value << std::endl;
        errno = 0;
        ssize_t t_written_size = ::send( f_socket, (void*)&a_value, sizeof( T ), flags );
        if( t_written_size == sizeof( T ) )
        {
            return t_written_size;
        }
        f_last_errno = errno;
        ret_errno = errno;
        if( t_written_size < 0 )
        {
            if( f_last_errno == EPIPE )
            {
                throw closed_connection() << "connection::send_type";
                return -1;
            }
            throw exception() << "send_type is unable to send; error message: " << strerror( f_last_errno );
            return -1;
        }
        throw exception() << "send_type is unable to write <" << a_value << ">; write size was different than value size\n";
        return t_written_size;
    }

    template< typename T >
    T connection::recv_type( int flags, int& ret_errno )
    {
        T t_value = T();
        errno = 0;
        ssize_t t_recv_size = ::recv( f_socket, (void*)&t_value, sizeof( T ), flags );
        if( t_recv_size > 0 )
        {
            //std::cout << "receiving something of size " << sizeof( T ) << "; size read: " << t_recv_size << "; value: " << t_value << std::endl;
            return t_value;
        }
        f_last_errno = errno;
        ret_errno = errno;
        if( t_recv_size == 0 && errno != EWOULDBLOCK && errno != EAGAIN )
        {
            throw closed_connection() << "connection::recv_type";
        }
        else if( t_recv_size < 0 )
        {
            throw exception() << "recv_type is unable to receive; error message: " << strerror( f_last_errno );
        }
        return t_value;
    }

}


#endif
