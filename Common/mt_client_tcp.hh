#ifndef MT_CLIENT_TCP_HH_
#define MT_CLIENT_TCP_HH_

#include "mt_connection.hh"

namespace mantis
{

    class client_tcp :
            public connection
    {
        public:
            client_tcp( const std::string& a_host, const int& a_port );
            virtual ~client_tcp();
    };

}

#endif
