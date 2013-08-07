#ifndef CLIENT_HH_
#define CLIENT_HH_

#include "connection.hh"

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class client
    {
        public:
            client();
            virtual ~client();

            void open( const std::string& a_host, const int& a_port );
            void close();

            connection* get_connection();

        private:
            connection* f_connection;
    };

}

#endif
