/*
 * mt_broker.cc
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_broker.hh"

#include "mt_connection.hh"
#include "mt_logger.hh"

#include "SimpleAmqpClient/AmqpLibraryException.h"

namespace mantis
{
    MTLOGGER( mtlog, "broker" );

    broker::broker( const std::string& a_address, unsigned a_port ) :
            f_address( a_address ),
            f_port( a_port )
    {
        // TODO check for broker
    }

    broker::~broker()
    {
    }

    connection* broker::create_connection()
    {
        connection* t_connection;
        try
        {
            t_connection = new connection( AmqpClient::Channel::Create( f_address, f_port ) );
        }
        catch( AmqpClient::AmqpLibraryException& e )
        {
            MTERROR( mtlog, "AMQP Library Exception caught: " << e.what() );
            return NULL;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Standard exception caught: " << e.what() );
            return NULL;
        }
        return t_connection;
    }

} /* namespace mantis */
