/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_client_config.hh"

#include<string>
using std::string;

namespace mantis
{

    client_config::client_config()
    {
        // default client configuration

        param_node* t_amqp_node = new param_node();
        t_amqp_node->add( "broker-port", param_value( 5672 ) );
        t_amqp_node->add( "broker", param_value( "localhost" ) );
        t_amqp_node->add( "exchange", param_value( "requests" ) );
        t_amqp_node->add( "route", param_value( "mantis" ) );
        add( "amqp", t_amqp_node );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
