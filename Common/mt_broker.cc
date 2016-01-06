/*
 * mt_broker.cc
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_broker.hh"

#include "mt_authentication.hh"

#include "logger.hh"
#include "param.hh"

#include "SimpleAmqpClient/AmqpLibraryException.h"

using scarab::param_node;

namespace mantis
{
    LOGGER( mtlog, "broker" );

    broker::broker( const std::string& a_address, unsigned a_port ) :
            f_address( a_address ),
            f_port( a_port )
    {
    }

    broker::~broker()
    {
    }

    amqp_channel_ptr broker::open_channel() const
    {
        try
        {
            authentication* t_auth = authentication::get_instance();
            if( ! t_auth->is_loaded() )
            {
                ERROR( mtlog, "Authentications were not loaded; create AMQP connection" );
                return AmqpClient::Channel::ptr_t();
            }
            const param_node* t_amqp_auth = t_auth->node_at( "amqp" );
            if( t_amqp_auth == NULL || ! t_amqp_auth->has( "username" ) || ! t_amqp_auth->has( "password" ) )
            {
                ERROR( mtlog, "AMQP authentication is not available or is not complete" );
                return AmqpClient::Channel::ptr_t();
            }
            return AmqpClient::Channel::Create( f_address, f_port, t_amqp_auth->get_value( "username" ), t_amqp_auth->get_value( "password" ) );
        }
        catch( AmqpClient::AmqpLibraryException& e )
        {
            ERROR( mtlog, "AMQP Library Exception caught: " << e.what() );
            return AmqpClient::Channel::ptr_t();
        }
        catch( std::exception& e )
        {
            ERROR( mtlog, "Standard exception caught: " << e.what() );
            return AmqpClient::Channel::ptr_t();
        }
    }

} /* namespace mantis */
