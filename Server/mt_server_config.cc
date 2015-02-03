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

        param_node* t_broker_node = new param_node();
        t_broker_node->add( "port", t_value << 5672 );
        t_broker_node->add( "addr", t_value << "localhost" );
        add( "broker", t_broker_node );

        param_node* t_run_node = new param_node();
        t_run_node->add( "duration", t_value << 500 );
        add( "run", t_run_node );

        param_node* t_dev_node = new param_node();
        t_dev_node->add( "name", t_value << "test" );
        t_dev_node->add( "rate", t_value << 250 );
        t_dev_node->add( "n-channels", t_value << 1 );
        t_dev_node->add( "data-mode", t_value << monarch3::sDigitized );
        t_dev_node->add( "channel-mode", t_value << monarch3::sInterleaved );
        t_dev_node->add( "sample-size", t_value << 1 );
        t_dev_node->add( "buffer-size", t_value << 512 );
        t_dev_node->add( "block-size", t_value << 4194304 );
        t_dev_node->add( "data-chunk-size", t_value << 1024 );
        // TODO: for multi-device mode, this node will be called "devices", and each device will have a separate node(?)
        add( "device", t_dev_node );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
