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

        param_value client_value;

        add( "port", client_value << 98342 );

        add( "host", client_value << "localhost" );

        add( "client-port", client_value << 98343 );

        add( "client-host", client_value << "localhost" );

        add( "file", client_value << "mantis_client_out.egg" );

        add( "rate", client_value << 250.0 );

        add( "duration", client_value << 1000 );

        add( "mode", client_value << 0 );

        add( "file-writer", client_value << "server" );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
