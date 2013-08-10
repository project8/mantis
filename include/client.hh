#ifndef CLIENT_HH_
#define CLIENT_HH_

#include "connection.hh"

namespace mantis
{

    class client :
            public connection
    {
        public:
            client( const std::string& a_host, const int& a_port );
            virtual ~client();
    };

}

#endif
