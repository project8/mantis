/*
 * mt_client_config.cc
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#include "mt_server_config.hh"

#include<string>
using std::string;

namespace Katydid
{

    server_config::server_config() :
            rapidjson::Document()
    {
        // default client configuration
        rapidjson::Value port( 98342 );
        rapidjson::Value host;
        host.SetString( "localhost", 9 );
        rapidjson::Value digitizer;
        digitizer.SetString( "px1500", 6 );

        AddMember( "port", port, GetAllocator() );
        AddMember( "host", host, GetAllocator() );
        AddMember( "digitizer", digitizer, GetAllocator() );
    }

    server_config::~server_config()
    {
    }

    standalone_config::standalone_config() :
            rapidjson::Document()
    {
        // default client configuration
        rapidjson::Value filename;
        filename.SetString( "mantis_client_out.egg", 21 );
        rapidjson::Value rate( 250.0 );
        rapidjson::Value duration( 1000 );
        rapidjson::Value digitizer;
        digitizer.SetString( "px1500", 6 );

        AddMember( "file", filename, GetAllocator() );
        AddMember( "rate", rate, GetAllocator() );
        AddMember( "duration", duration, GetAllocator() );
        AddMember( "digitizer", digitizer, GetAllocator() );
    }

    standalone_config::~standalone_config()
    {
    }

} /* namespace Katydid */
