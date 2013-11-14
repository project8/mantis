#ifndef MT_CONNECTION_HH_
#define MT_CONNECTION_HH_

#include <netinet/in.h>
#include <string>
#include <sys/types.h>

namespace mantis
{

    class connection
    {
        public:
            connection( int a_socket, sockaddr_in* an_address );
            virtual ~connection();

            ssize_t send( const char* a_message, size_t a_size, int flags = 0 );
            ssize_t recv( char* a_message, size_t a_size, int flags = 0 );

            size_t recv_size( int flags = 0 );

        protected:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
