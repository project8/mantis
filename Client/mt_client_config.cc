/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_client_config.hh"

#include<string>
using std::string;

namespace mantis
{

    client_config::client_config()
    {
        // default client configuration

        param_value t_value;

        param_node* t_broker_node = new param_node();
        t_broker_node->add( "port", t_value << 98342 );
        t_broker_node->add( "addr", t_value << "localhost" );
        add( "broker", t_broker_node );

        param_node* t_file_node = new param_node();
        t_file_node->add( "filename", t_value << "mantis_client_out.egg" );
        t_file_node->add( "description", t_value << "???" );
        add( "file", t_file_node );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
