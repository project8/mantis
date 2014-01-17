/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_client_config.hh"

#include<string>
using std::string;

#include<iostream>

namespace mantis
{

    client_config::client_config()
    {
        // default client configuration

        config_value_data client_data;

        add( "port", client_data << 98342 );

        add( "host", client_data << "localhost" );

        add( "client-port", client_data << 98343 );

        add( "client-host", client_data << "localhost" );

        add( "file", client_data << "mantis_client_out.egg" );

        add( "rate", client_data << 250.0 );

        add( "duration", client_data << 1000 );

        add( "mode", client_data << 0 );

        add( "file-writer", client_data << "server" );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
