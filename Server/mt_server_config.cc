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

using scarab::param_node;
using scarab::param_value;

namespace mantis
{

    server_config::server_config()
    {
        // default server configuration

        param_node* t_amqp_node = new param_node();
        t_amqp_node->add( "broker-port", new param_value( 5672 ) );
        t_amqp_node->add( "broker", new param_value( "localhost" ) );
        t_amqp_node->add( "exchange", new param_value( "requests" ) );
        t_amqp_node->add( "alert-exchange", new param_value( "alerts" ) );
        t_amqp_node->add( "queue", new param_value( "mantis" ) );
        //t_amqp_node->add( "file-finished-key", new param_value( "completed_files" ) );
        add( "amqp", t_amqp_node );

        param_node* t_acq_node = new param_node();
        t_acq_node->add( "devices", new param_node() );
        t_acq_node->add( "duration", new param_value( 500 ) );
        add( "acq", t_acq_node );

    }

    server_config::~server_config()
    {
    }

} /* namespace mantis */
