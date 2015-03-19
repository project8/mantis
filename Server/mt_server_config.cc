/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS

#include "mt_server_config.hh"

#include "M3Types.hh"

#include<string>
using std::string;

namespace mantis
{

    server_config::server_config()
    {
        // default server configuration

        param_value t_value;

        param_node* t_amqp_node = new param_node();
        t_amqp_node->add( "broker-port", t_value << 5672 );
        t_amqp_node->add( "broker", t_value << "localhost" );
        t_amqp_node->add( "exchange", t_value << "requests" );
        t_amqp_node->add( "route", t_value << "mantis" );
        add( "amqp", t_amqp_node );

        param_node* t_run_node = new param_node();
        t_run_node->add( "duration", t_value << 500 );
        add( "run", t_run_node );

        add( "devices", new param_node() );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
