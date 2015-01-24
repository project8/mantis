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

        param_value server_config;

        add( "broker-port", server_config << 98342 );

        add( "broker-addr", server_config << "localhost" );

        add( "digitizer", server_config << "test" );

        add( "buffer-size", server_config << 512 );

        add( "block-size", server_config << 4194304 );

        add( "data-chunk-size", server_config << 1024 );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
