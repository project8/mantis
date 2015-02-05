#ifndef MT_CONNECTION_HH_
#define MT_CONNECTION_HH_

#include "mt_constants.hh"

#include "SimpleAmqpClient/SimpleAmqpClient.h"

namespace mantis
{
    class MANTIS_API connection
    {
        public:
            connection( AmqpClient::Channel::ptr_t a_chan );
            ~connection();

            AmqpClient::Channel::ptr_t amqp();

        private:
            AmqpClient::Channel::ptr_t f_amqp_chan;

    };
}

#endif /* MT_CONNECTION_HH_ */
