#ifndef SERVER_SOCKET_HH_
#define SERVER_SOCKET_HH_

#include "connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server_socket
    {
        public:
            server_socket();
            virtual ~server_socket();

            void open( const int& a_port );
            void close();

            connection* get_connection();

        private:
            int f_socket;
            sockaddr_in* f_address;

            connection* f_connections[16];
            int f_index;
    };

}

#endif
