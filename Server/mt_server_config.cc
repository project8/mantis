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

        param_data server_config;

        add( "port", server_config << 98342 );

        add( "digitizer", server_config << "px1500" );

        add( "buffer-size", server_config << 512 );

        add( "record-size", server_config << 4194304 );

        add( "data-chunk-size", server_config << 1024 );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
