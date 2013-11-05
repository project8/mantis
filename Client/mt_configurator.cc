/*
 * mt_configurator.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "mt_configurator.hh"

#include "mt_parser.hh"

#include "filestream.h"

#include <cstdio>

using std::string;

namespace mantis
{

    configurator::configurator( int an_argc, char** an_argv, configuration a_default ) :
            f_master_config()
    {
        parser t_parser( an_argc, an_argv );

        // first configuration: defaults
        f_master_config += a_default;

        string t_name_config("config");
        string t_name_json("json");

        // second configuration: config file
        string t_config_filename = t_parser.get_optional( t_name_config, string() );
        if ( ! t_config_filename.empty() )
        {
            FILE* config_file = fopen( t_config_filename.c_str(), "r" );
            rapidjson::FileStream config_stream( config_file );
            if( config_file != NULL )
            {
                throw exception() << "config file <" << t_config_filename << "> did not open";
            }

            configuration t_second_config;
            t_second_config.ParseStream<0>( config_stream );
            // TODO: detect error in parsing
            fclose( config_file );

            f_master_config += t_second_config;
        }

        // third configuration: command line json
        string t_config_json = t_parser.get_optional( t_name_json, string() );
        if( ! t_config_filename.empty() )
        {
            configuration t_third_config;
            t_third_config.Parse<0>( t_config_json.c_str() );
            // TODO: detect error in parsing

            f_master_config += t_third_config;
        }

        // fourth configuration: command line arguments
        for( parser::imap_cit iter = t_parser.begin(); iter != t_parser.end(); ++iter )
        {
            if ( iter->first == t_name_config || iter->first == t_name_json )
                continue;

            f_master_config.AddMember( iter->first.c_str(), iter->second.c_str(), f_master_config.GetAllocator() );
        }
    }

    configurator::~configurator()
    {
    }

} /* namespace mantis */
