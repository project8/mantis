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

            void write( const std::string& a_message );
            void read( std::string& a_message );

        protected:
            int f_socket;
            sockaddr_in* f_address;

            static const int f_buffer_length = 1024;
            char f_buffer_content[ f_buffer_length ];
    };

}

#endif
