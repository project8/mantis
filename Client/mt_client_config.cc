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
        rapidjson::Value port( 98342 );
        AddMember( "port", port, GetAllocator() );

        rapidjson::Value host;
        host.SetString( "localhost", 9 );
        AddMember( "host", host, GetAllocator() );

        rapidjson::Value filename;
        filename.SetString( "mantis_client_out.egg", 21 );
        AddMember( "file", filename, GetAllocator() );

        rapidjson::Value rate( 250.0 );
        AddMember( "rate", rate, GetAllocator() );

        rapidjson::Value duration( 1000 );
        AddMember( "duration", duration, GetAllocator() );

        rapidjson::Value mode( 0 );
        AddMember( "mode", mode, GetAllocator() );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
