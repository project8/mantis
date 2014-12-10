#ifndef MT_SERVER_UDP_HH_
#define MT_SERVER_UDP_HH_

#include <netinet/in.h>
#include <string>

namespace mantis
{

    class server_udp
    {
        public:
            server_udp( const int& a_port );
            virtual ~server_udp();

            ssize_t recvfrom( char* a_message, size_t a_size, int flags = 0, int& ret_errno = f_last_errno );

            sockaddr_in* client_address();

        private:
            int f_socket;
            sockaddr_in* f_address;
            sockaddr_in* f_client_address;
            socklen_t f_client_address_length;

        private:
            static int f_last_errno;
};

}

#endif
