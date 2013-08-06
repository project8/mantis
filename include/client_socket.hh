#ifndef CLIENT_SOCKET_HH_
#define CLIENT_SOCKET_HH_

#include "connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class client_socket
    {
        public:
            client_socket();
            virtual ~client_socket();

            void open( const std::string& a_host, const int& a_port );
            void close();

            connection* get_connection();

        private:
            connection* f_connection;
    };

}

#endif
