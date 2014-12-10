#ifndef MT_CLIENT_UDP_HH_
#define MT_CLIENT_UDP_HH_

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class client_udp
    {
        public:
            client_udp( const std::string& a_host, const int& a_port );
            virtual ~client_udp();

            ssize_t send( const char* a_message, size_t a_size, int flags = 0, int& ret_errno = f_last_errno );

        protected:
            int f_socket;
            sockaddr_in* f_address;

        private:
            static int f_last_errno;
    };

}

#endif
