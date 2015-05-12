#define MANTIS_API_EXPORTS

#include "mt_connection.hh"

#include "SimpleAmqpClient/AmqpLibraryException.h"

namespace mantis
{
    connection::connection( AmqpClient::Channel::ptr_t a_chan ) :
            f_amqp_chan( a_chan )
    {
    }

    connection::~connection()
    {
    }

    AmqpClient::Channel::ptr_t connection::amqp()
    {
        return f_amqp_chan;
    }

}
