#ifndef SERVER_HH_
#define SERVER_HH_

#include "mt_connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server
    {
        public:
            server( const int& a_port );
            virtual ~server();

            connection* get_connection();

        private:
            int f_socket;
            sockaddr_in* f_address;
    };

}

#endif
