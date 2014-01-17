/*
 * mt_configurator.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "mt_configurator.hh"

#include "mt_parser.hh"

#include <iostream>
using std::cout;
using std::endl;

using std::string;

namespace mantis
{

    configurator::configurator( int an_argc, char** an_argv, config_value_object* a_default ) :
            f_master_config(),
            f_config_value_buffer( NULL ),
            f_string_buffer()
    {
        parser t_parser( an_argc, an_argv );
        //std::cout << "options parsed" << std::endl;
        //cout << t_parser << endl;

        // first configuration: defaults
        if ( a_default != NULL )
        {
            f_master_config.merge(a_default);
        }

        //std::cout << "first configuration complete" << std::endl;
        //cout << f_master_config << endl;
        //cout << t_parser << endl;

        string t_name_config("config");
        string t_name_json("json");

        // second configuration: config file
        if( t_parser.has( t_name_config ) )
        {
            string t_config_filename = t_parser.data_at(t_name_config)->value();
            if( ! t_config_filename.empty() )
            {
                config_value_object* t_config_from_file = config_maker_json::read_file( t_config_filename );
                f_master_config.merge( t_config_from_file );
                delete t_config_from_file;
            }
        }

        //std::cout << "second configuration complete" << std::endl;
        //cout << f_master_config << endl;
        //cout << t_parser << endl;

        // third configuration: command line json
        if( t_parser.has( t_name_json ) )
        {
            string t_config_json = t_parser.data_at(t_name_json)->value();
            if( ! t_config_json.empty() )
            {
                config_value_object* t_config_from_json = config_maker_json::read_string( t_config_json );
                f_master_config.merge( t_config_from_json );
                delete t_config_from_json;
            }
        }

        //std::cout << "third configuration complete" << std::endl;
        //cout << f_master_config << endl;
        //cout << t_parser << endl;

        // fourth configuration: command line arguments
        t_parser.erase( t_name_config );
        t_parser.erase( t_name_json );

        //std::cout << "removed config and json from parsed options" << std::endl;
        //cout << t_parser << endl;
        f_master_config.merge( &t_parser );

        //std::cout << "fourth configuration complete" << std::endl;
        cout << "[configurator] final configuration:" << endl;
        cout << f_master_config << endl;

    }

    configurator::~configurator()
    {
    }

    config_value_object& configurator::config()
    {
        return f_master_config;
    }

    const config_value_object& configurator::config() const
    {
        return f_master_config;
    }

} /* namespace mantis */
