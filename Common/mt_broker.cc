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

    broker::broker() :
            f_address(),
            f_port(),
            f_connection( NULL )
    {
    }

    broker::~broker()
    {
        delete f_connection;
    }

    bool broker::connect( const std::string& a_address, unsigned a_port )
    {
        if( f_connection == NULL )
        {
            MTERROR( mtlog, "Broker is already connected to " << f_address << ":" << f_port );
            return false;
        }

        connection* t_connection = NULL;
        try
        {
            t_connection = new connection( AmqpClient::Channel::Create( a_address, a_port ) );
        }
        catch( AmqpClient::AmqpLibraryException& e )
        {
            MTERROR( mtlog, "AMQP Library Exception caught: " << e.what() );
            return false;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Standard exception caught: " << e.what() );
            return false;
        }

        f_address = a_address;
        f_port = a_port;
        f_connection = t_connection;

        return true;
    }

    void broker::disconnect()
    {
        if( f_connection == NULL )
        {
            return;
        }

        delete f_connection;
        f_address.clear();
        f_port = 0;

        return;
    }

} /* namespace mantis */
