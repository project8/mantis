#ifndef MT_CLIENT_HH_
#define MT_CLIENT_HH_

#include "mt_connection.hh"

namespace mantis
{

    class client : public connection
    {
        public:
            client( const std::string& a_host, const int& a_port, socket_type a_type );
            virtual ~client();
    };

}

#endif
