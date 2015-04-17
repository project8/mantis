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
        // deserialize the buffer
        msgpack::unpacked result;
        msgpack::unpack(result, a_msgpack_string.data(), a_msgpack_string.size());
        msgpack::object deserialized = result.get();
        MTDEBUG( mtlog, "msgpack deserialization result is:\n" << deserialized );
        MTDEBUG( mtlog, "with size: " << deserialized.via.array.size );
        MTWARN( mtlog, "deserialized type is " << deserialized.type );

        
        // convert it to a param node
        if ( deserialized.type != 7 ) // message payload should deserialize to a map
        {
            return NULL;
        }
        param_node* t_config = new param_node();
        for( unsigned iElement=0; iElement < deserialized.via.array.size; ++iElement)
        {
            t_config->replace( deserialized.via.array.ptr[2*iElement].as<std::string>(), 
                               param_input_msgpack::read_msgpack_element( deserialized.via.array.ptr[2*iElement+1] ));
            MTDEBUG( mtlog, "config is now:" << std::endl << *t_config );
        }

        return t_config;
    }

    param* param_input_msgpack::read_msgpack_element( const msgpack::object& a_msgpack_element )
    {
        MTDEBUG( mtlog, "case will be: " << a_msgpack_element.type );
        switch ( a_msgpack_element.type ){
            case 0: // NULL
                return new param();
                break;
            case 1: // BOOL
                return new param_value( a_msgpack_element.as<bool>() );
                break;
            case 2: // positive int
                return new param_value( a_msgpack_element.as<unsigned>() );
                break;
            case 3: // negative int
                return new param_value( a_msgpack_element.as<int>() );
                break;
            case 4: // float
                return new param_value( a_msgpack_element.as<float>() );
                break;
            case 5: // string
                return new param_value( a_msgpack_element.as<std::string>() );
                break;
            case 6: // array
                {
                    param_array* t_config_array = new param_array();
                    for( unsigned iElement=0; iElement < a_msgpack_element.via.array.size; ++iElement )
                    {
                        t_config_array->push_back( param_input_msgpack::read_msgpack_element(a_msgpack_element.via.array.ptr[iElement]) );
                    }
                    return t_config_array;
                    break;
                }
            case 7: // map
                {
                    param_node* t_config_node = new param_node();
                    for( unsigned iElement=0; iElement < a_msgpack_element.via.array.size; ++iElement)
                    {
                        t_config_node->replace( a_msgpack_element.via.array.ptr[2*iElement].as<std::string>(), 
                                                param_input_msgpack::read_msgpack_element( a_msgpack_element.via.array.ptr[2*iElement+1] ));
                    }
                    return t_config_node;
                    break;
                }
            case 8: // BIN
                MTWARN( mtlog, "not sure how to deal with a 'bin'" );
                return NULL;
                break;
            default:
                MTWARN( mtlog, "type unrecognized: " << a_msgpack_element.type );
                return NULL;
                break;
        }
        MTWARN( mtlog, "something should have returned before ever getting here, something went wrong" );
        return NULL;
    }

    param_output_msgpack::param_output_msgpack()
    {}

    param_output_msgpack::~param_output_msgpack()
    {}

    bool param_output_msgpack::write_string( const param& /*a_to_write*/, std::string& /*a_string*/, json_writing_style /*a_style*/ )
    {
        return true;
    }

} /* namespace mantis */
