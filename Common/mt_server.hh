#ifndef MT_SERVER_HH_
#define MT_SERVER_HH_

#include "mt_connection.hh"

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace mantis
{

    class server
    {
        public:
            server( const int& a_port, socket_type a_type );
            virtual ~server();

            connection* get_connection();

        private:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
