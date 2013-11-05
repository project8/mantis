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
        rapidjson::Value port( 98342 );
        rapidjson::Value filename;
        filename.SetString( "mantis_client_out.egg", 21 );
        rapidjson::Value rate( 250.0 );
        rapidjson::Value duration( 1000 );

        AddMember( "port", port, GetAllocator() );
        AddMember( "file", filename, GetAllocator() );
        AddMember( "rate", rate, GetAllocator() );
        AddMember( "duration", duration, GetAllocator() );
    }

    client_config::~client_config()
    {
    }

} /* namespace Katydid */
