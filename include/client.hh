#ifndef CLIENT_HH_
#define CLIENT_HH_

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class client
    {
        public:
            client( const std::string& a_host, const int& a_port );
            virtual ~client();

            void write( const std::string& a_message );
            void read( std::string& a_message );

        private:
            int f_socket;
            sockaddr_in* f_address;

            static const int f_buffer_length = 512;
            char f_buffer_content[ f_buffer_length ];
    };

}

#endif
