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

        add( "port", server_config << 98342 );

        add( "digitizer", server_config << "test" );

        add( "buffer-size", server_config << 512 );

        add( "record-size", server_config << 4194304 );

        add( "data-chunk-size", server_config << 1024 );
        
        add( "roach-host", server_config << "198.125.161.179" );
        
        add( "roach-boffile", server_config << "adc.bof" );
    }

    server_config::~server_config()
    {
    }

} /* namespace Katydid */
