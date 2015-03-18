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

        param_value t_value;

        param_node* t_amqp_node = new param_node();
        t_amqp_node->add( "broker-port", t_value << 5672 );
        t_amqp_node->add( "broker", t_value << "localhost" );
        t_amqp_node->add( "exchange", t_value << "requests" );
        t_amqp_node->add( "route", t_value << "mantis" );
        add( "amqp", t_amqp_node );

        param_node* t_file_node = new param_node();
        t_file_node->add( "filename", t_value << "mantis_client_out.egg" );
        t_file_node->add( "description", t_value << "???" );
        add( "file", t_file_node );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
