#ifndef MT_CLIENT_UDP_HH_
#define MT_CLIENT_UDP_HH_

#include "mt_connection.hh"

namespace mantis
{

    class client_udp :
            public connection
    {
        public:
            client_udp( const std::string& a_host, const int& a_port );
            virtual ~client_udp();
    };

}

#endif
