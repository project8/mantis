#ifndef CONNECTION_HH_
#define CONNECTION_HH_

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class connection
    {
        public:
            connection( const int& a_socket, const sockaddr_in* an_address );
            virtual ~connection();

            void write( const std::string& a_message );
            void read( std::string& a_message );

        protected:
            int f_socket;
            sockaddr_in* f_address;

            static const int f_buffer_length = 512;
            char f_buffer_content[ f_buffer_length ];
    };

}

#endif
