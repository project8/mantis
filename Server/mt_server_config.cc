/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_server_config.hh"

#include<string>
using std::string;

namespace mantis
{

    server_config::server_config()
    {
        // default server configuration

        param_value t_value;

        add( "broker-port", t_value << 98342 );

        add( "broker-addr", t_value << "localhost" );

        add( "duration", t_value << 500 );

        param_node* t_dev_node = new param_node();
        t_dev_node->add( "name", t_value << "test" );
        t_dev_node->add( "rate", t_value << 250.0 );
        t_dev_node->add( "mode", t_value << 0 );
        t_dev_node->add( "buffer-size", t_value << 512 );
        t_dev_node->add( "block-size", t_value << 4194304 );
        t_dev_node->add( "data-chunk-size", t_value << 1024 );
        add( "device", t_dev_node );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
