/*
 * mt_configurator.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "mt_configurator.hh"

#include "mt_parser.hh"
#include "mt_logger.hh"

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "configurator" );

    configurator::configurator( int an_argc, char** an_argv, param_node* a_default ) :
            f_master_config(),
            f_param_buffer( NULL ),
            f_string_buffer()
    {
        parser t_parser( an_argc, an_argv );
        //std::cout << "options parsed" << std::endl;
        //cout << t_parser );

        // first configuration: defaults
        if ( a_default != NULL )
        {
            f_master_config.merge(a_default);
        }

        //std::cout << "first configuration complete" << std::endl;
        //cout << f_master_config );
        //cout << t_parser );

        string t_name_config("config");
        string t_name_json("json");

        // second configuration: config file
        if( t_parser.has( t_name_config ) )
        {
            string t_config_filename = t_parser.value_at( t_name_config )->get();
            if( ! t_config_filename.empty() )
            {
                param_node* t_config_from_file = param_input_json::read_file( t_config_filename );
                if( t_config_from_file == NULL )
                {
                    throw exception() << "[configurator] error parsing config file";
                }
                f_master_config.merge( t_config_from_file );
                delete t_config_from_file;
            }
        }

        //std::cout << "second configuration complete" << std::endl;
        //cout << f_master_config );
        //cout << t_parser );

        // third configuration: command line json
        if( t_parser.has( t_name_json ) )
        {
            string t_config_json = t_parser.value_at( t_name_json )->get();
            if( ! t_config_json.empty() )
            {
                param_node* t_config_from_json = param_input_json::read_string( t_config_json );
                f_master_config.merge( t_config_from_json );
                delete t_config_from_json;
            }
        }

        //std::cout << "third configuration complete" << std::endl;
        //cout << f_master_config );
        //cout << t_parser );

        // fourth configuration: command line arguments
        t_parser.erase( t_name_config );
        t_parser.erase( t_name_json );

        //std::cout << "removed config and json from parsed options" << std::endl;
        //cout << t_parser );
        f_master_config.merge( &t_parser );

        //std::cout << "fourth configuration complete" << std::endl;
        MTINFO( mtlog, "final configuration:\n" << f_master_config );

    }

    configurator::~configurator()
    {
    }

    param_node& configurator::config()
    {
        return f_master_config;
    }

    const param_node& configurator::config() const
    {
        return f_master_config;
    }

} /* namespace mantis */
