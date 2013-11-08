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
        // default client configuration
        rapidjson::Value port( 98342 );
        rapidjson::Value digitizer;
        digitizer.SetString( "px1500", 6 );
        rapidjson::Value buffer_size( 512 );
        rapidjson::Value record_size( 4194304 );

        AddMember( "port", port, GetAllocator() );
        AddMember( "digitizer", digitizer, GetAllocator() );
        AddMember( "buffer-size", buffer_size, GetAllocator() );
        AddMember( "record-size", record_size, GetAllocator() );
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
        rapidjson::Value buffer_size( 512 );
        rapidjson::Value record_size( 4194304 );

        AddMember( "file", filename, GetAllocator() );
        AddMember( "rate", rate, GetAllocator() );
        AddMember( "duration", duration, GetAllocator() );
        AddMember( "digitizer", digitizer, GetAllocator() );
        AddMember( "buffer-size", buffer_size, GetAllocator() );
        AddMember( "record-size", record_size, GetAllocator() );
    }

    standalone_config::~standalone_config()
    {
    }

} /* namespace Katydid */
