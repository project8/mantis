#ifndef MT_SERVER_UDP_HH_
#define MT_SERVER_UDP_HH_

#include "mt_connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server_udp
    {
        public:
            server_udp( const int& a_port );
            virtual ~server_udp();

            connection* get_connection();

        private:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
