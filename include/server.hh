#ifndef SERVER_HH_
#define SERVER_HH_

#include "connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server
    {
        public:
            server();
            virtual ~server();

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
