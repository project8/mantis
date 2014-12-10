#ifndef MT_SERVER_TCP_HH_
#define MT_SERVER_TCP_HH_

#include "mt_connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server_tcp
    {
        public:
            server_tcp( const int& a_port );
            virtual ~server_tcp();

            connection* get_connection();

        private:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
