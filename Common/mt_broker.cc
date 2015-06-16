/*
 * mt_broker.cc
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_broker.hh"

#include "mt_authentication.hh"
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
        if( f_connection != NULL )
        {
            disconnect();
        }
    }

    bool broker::connect( const std::string& a_address, unsigned a_port )
    {
        if( f_connection != NULL )
        {
            MTERROR( mtlog, "Broker is already connected to " << f_address << ":" << f_port );
            return false;
        }

        connection* t_connection = NULL;
        try
        {
            authentication* t_auth = authentication::get_instance();
            if( ! t_auth->is_loaded() )
            {
                MTERROR( mtlog, "Authentications were not loaded; create AMQP connection" );
                return false;
            }
            const param_node* t_amqp_auth = t_auth->node_at( "amqp" );
            if( t_amqp_auth == NULL || ! t_amqp_auth->has( "username" ) || ! t_amqp_auth->has( "password" ) )
            {
                MTERROR( mtlog, "AMQP authentication is not available or is not complete" );
                return false;
            }
            t_connection = new connection( AmqpClient::Channel::Create( a_address, a_port, t_amqp_auth->get_value( "username" ), t_amqp_auth->get_value( "password" ) ) );
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

        MTDEBUG( mtlog, "Connected to AMQP broker" );

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

        MTDEBUG( mtlog, "Disconnecting from AMQP broker" );
        delete f_connection;
        f_connection = NULL;
        f_address.clear();
        f_port = 0;

        return;
    }

} /* namespace mantis */
