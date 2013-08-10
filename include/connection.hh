#ifndef CONNECTION_HH_
#define CONNECTION_HH_

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class connection
    {
        public:
            connection( int a_socket, sockaddr_in* an_address );
            virtual ~connection();

            void write( const char* a_message, size_t a_size );
            void read( char* a_message, size_t a_size );

        protected:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
