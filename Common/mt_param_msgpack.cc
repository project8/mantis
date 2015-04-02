/*
 * mt_param_msgpack.cc
 *
 *  Created on: March 27, 2015
 *      Author: bhlaroque
 */

#define MANTIS_API_EXPORTS

#include "msgpack.hpp"

#include "mt_param_msgpack.hh"
#include "mt_param_json.hh"

#include "mt_logger.hh"

#include <sstream>
using std::string;
using std::stringstream;


#include <cstdio>


namespace mantis
{
    MTLOGGER( mtlog, "param" );

    param_input_msgpack::param_input_msgpack()
    {
    }
    param_input_msgpack::~param_input_msgpack()
    {
    }

    param_node* param_input_msgpack::read_string( const std::string& a_msgpack_string )
    {
        //param_node* t_config = new param_node();

        msgpack::unpacked result;
        msgpack::unpack(result, a_msgpack_string.data(), a_msgpack_string.size());
        msgpack::object deserialized = result.get();
        MTDEBUG( mtlog, "msgpack deserialization result is:\n" << deserialized );


        msgpack::object_array deserialized_array = deserialized.via.array;
        param_node* t_config = param_input_msgpack::read_msgpack_array( deserialized_array );

        return t_config;
    }

    param_node* param_input_msgpack::read_msgpack_array( const msgpack::object_array& a_msgpack_array )
    {
        param_node* t_config = new param_node();

        for (unsigned Iparam=0; Iparam < a_msgpack_array.size; ++Iparam)
        {
//            MTDEBUG( mtlog, "param_node so far is: " << *t_config );
//            MTDEBUG( mtlog, "param " << Iparam << " addition -> " << a_msgpack_array.ptr[2*Iparam] << " : " << a_msgpack_array.ptr[2*Iparam+1] );
//            MTDEBUG( mtlog, "case will be " << a_msgpack_array.ptr[2*Iparam+1].type);
            switch ( a_msgpack_array.ptr[2*Iparam+1].type ){
                case 0: // NULL
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), new param() );
                    break;
                case 1: // BOOL
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_value(a_msgpack_array.ptr[2*Iparam+1].as<bool>()) );
                    break;
                case 2: // positive int
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_value(a_msgpack_array.ptr[2*Iparam+1].as<unsigned>()) );
                    break;
                case 3: // negative int
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_value(a_msgpack_array.ptr[2*Iparam+1].as<int>()) );
                    break;
                case 4: // float
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_value(a_msgpack_array.ptr[2*Iparam+1].as<float>()) );
                    break;
                case 5: // string
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_value(a_msgpack_array.ptr[2*Iparam+1].as<std::string>()) );
                    break;
                case 6: // array
                    t_config->add( a_msgpack_array.ptr[2*Iparam].as<std::string>(), param_input_json::read_string(a_msgpack_array.ptr[2*Iparam+1].via.array.ptr[0].as<std::string>()));
                    #include "mt_param_json.hh"
                    break;
                case 7: // map
                case 8: // BIN
                default:
                    MTWARN( mtlog, "type unrecognized: " << a_msgpack_array.ptr[2*Iparam+1].type );
                    break;
            }
        }
        MTDEBUG( mtlog, "returning: " << *t_config );
        return t_config;
    }

    param_output_msgpack::param_output_msgpack()
    {}

    param_output_msgpack::~param_output_msgpack()
    {}

    bool param_output_msgpack::write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style )
    {
        return true;
    }

} /* namespace mantis */
