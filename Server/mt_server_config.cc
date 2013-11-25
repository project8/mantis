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
        AddMember( "port", port, GetAllocator() );

        rapidjson::Value digitizer;
        digitizer.SetString( "px1500", 6 );
        AddMember( "digitizer", digitizer, GetAllocator() );

        rapidjson::Value buffer_size( 512 );
        AddMember( "buffer-size", buffer_size, GetAllocator() );

        rapidjson::Value record_size( 4194304 );
        AddMember( "record-size", record_size, GetAllocator() );

	rapidjson::Value data_chunk_size( 1024 );
        AddMember( "data-chunk-size", data_chunk_size, GetAllocator() );
    }

    server_config::~server_config()
    {
    }

    standalone_config::standalone_config()
    {
        // default client configuration
        rapidjson::Value filename;
        filename.SetString( "mantis_standalone_out.egg", 21 );
        AddMember( "file", filename, GetAllocator() );

        rapidjson::Value rate( 250.0 );
        AddMember( "rate", rate, GetAllocator() );

        rapidjson::Value duration( 1000 );
        AddMember( "duration", duration, GetAllocator() );

        rapidjson::Value mode( 0 );
        AddMember( "mode", mode, GetAllocator() );

        rapidjson::Value digitizer;
        digitizer.SetString( "px1500", 6 );
        AddMember( "digitizer", digitizer, GetAllocator() );

        rapidjson::Value buffer_size( 512 );
        AddMember( "buffer-size", buffer_size, GetAllocator() );

        rapidjson::Value record_size( 4194304 );
        AddMember( "record-size", record_size, GetAllocator() );
    }

    standalone_config::~standalone_config()
    {
    }

} /* namespace Katydid */
