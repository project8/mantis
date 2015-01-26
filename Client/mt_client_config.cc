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

        add( "broker-port", t_value << 98342 );

        add( "broker-addr", t_value << "localhost" );

        add( "file", t_value << "mantis_client_out.egg" );

        add( "description", t_value << "???" );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
