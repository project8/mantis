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

#include <iostream>

namespace mantis
{

    configurator::configurator( int an_argc, char** an_argv, configuration* a_default ) :
            f_master_config(),
            f_string_buffer()
    {
        parser t_parser( an_argc, an_argv );
        std::cout << "options parsed" << std::endl;
        t_parser.print();

        // first configuration: defaults
        if ( a_default != NULL )
        {
            f_master_config += *a_default;
        }

        std::cout << "first configuration complete" << std::endl;
        f_master_config.print();
        t_parser.print();

        string t_name_config("config");
        string t_name_json("json");

        // second configuration: config file

        rapidjson::Value& t_config_filename = t_parser[t_name_config.c_str()];
        if ( t_config_filename.IsString() )
        {
            FILE* config_file = fopen( t_config_filename.GetString(), "r" );
            if( config_file == NULL )
            {
                std::cerr << "file <" << t_config_filename.GetString() << "> did not open" << std::endl;
                throw exception() << "config file <" << t_config_filename.GetString() << "> did not open";
            }

            rapidjson::FileStream config_stream( config_file );

            configuration t_second_config;
            if( t_second_config.ParseStream<0>( config_stream ).HasParseError() )
            {
                std::cerr << "error parsing config file:\n" << t_second_config.GetParseError() << std::endl;
                throw exception() << "error parsing config file";
            }
            fclose( config_file );

            f_master_config += t_second_config;
        }

        std::cout << "second configuration complete" << std::endl;
        f_master_config.print();
        t_parser.print();

        // third configuration: command line json
        rapidjson::Value& t_config_json = t_parser[t_name_json.c_str()];
        if( t_config_json.IsString() )
        {
            configuration t_third_config;
            if( t_third_config.Parse<0>( t_config_json.GetString() ).HasParseError() )
            {
                std::cerr << "error parsing command-line json: " << t_third_config.GetParseError() << std::endl;
                throw exception() << "error parsing command-line json";
            }
            std::cout << "command-line json:" << std::endl;
            t_third_config.print();

            f_master_config += t_third_config;
        }

        std::cout << "third configuration complete" << std::endl;
        f_master_config.print();
        t_parser.print();

        // fourth configuration: command line arguments
        t_parser.RemoveMember( t_name_config.c_str() );
        t_parser.RemoveMember( t_name_json.c_str() );
        std::cout << "removed config and json from parsed options" << std::endl;
        t_parser.print();
        f_master_config += t_parser;

        std::cout << "fourth configuration complete" << std::endl;
        f_master_config.print();

    }

    configurator::~configurator()
    {
    }

    configuration& configurator::config()
    {
        return f_master_config;
    }

    const configuration& configurator::config() const
    {
        return f_master_config;
    }

    bool configurator::get_bool_required( const std::string& a_name )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetBool();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }
    bool configurator::get_bool_optional( const std::string& a_name, bool a_default )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetBool();
        }
        return a_default;
    }

    int configurator::get_int_required( const std::string& a_name )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetInt();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }
    int configurator::get_int_optional( const std::string& a_name, int a_default )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetInt();
        }
        return a_default;
    }

    unsigned configurator::get_uint_required( const std::string& a_name )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetUint();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }
    unsigned configurator::get_uint_optional( const std::string& a_name, unsigned a_default )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetUint();
        }
        return a_default;
    }

    double configurator::get_double_required( const std::string& a_name )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetDouble();
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }
    double configurator::get_double_optional( const std::string& a_name, double a_default )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_master_config[ a_name.c_str() ].GetDouble();
        }
        return a_default;
    }

    const std::string& configurator::get_string_required( const std::string& a_name )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_string_buffer.assign( f_master_config[ a_name.c_str() ].GetString() );
        }
        throw exception() << "configurator does not have a value for <" << a_name << ">";
    }
    const std::string& configurator::get_string_optional( const std::string& a_name, const std::string& a_default )
    {
        if( f_master_config.HasMember( a_name.c_str() ) )
        {
            return f_string_buffer.assign( f_master_config[ a_name.c_str() ].GetString() );
        }
        return f_string_buffer.assign( a_default );
    }



} /* namespace mantis */
