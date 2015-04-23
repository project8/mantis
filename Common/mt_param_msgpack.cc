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
        msgpack::unpacked t_unpacked;
        msgpack::unpack( t_unpacked, a_msgpack_string.data(), a_msgpack_string.size()) ;
        msgpack::object t_deserialized = t_unpacked.get();
        MTDEBUG( mtlog, "msgpack deserialization result is: (size = " << t_deserialized.via.array.size << ")\n" << t_deserialized );

        
        // convert it to a param node
        if( t_deserialized.type != 7 ) // message payload should deserialize to a map
        {
            return NULL;
        }
        param_node* t_config = new param_node();
        for( unsigned i_element=0; i_element < t_deserialized.via.array.size; ++i_element )
        {
            t_config->replace( t_deserialized.via.array.ptr[2 * i_element].as<std::string>(),
                               param_input_msgpack::read_msgpack_element( t_deserialized.via.array.ptr[2 * i_element + 1] ) );
        }
        MTDEBUG( mtlog, "Unpacked string:\n" << *t_config );

        return t_config;
    }

    param* param_input_msgpack::read_msgpack_element( const msgpack::object& a_msgpack_element )
    {
        MTDEBUG( mtlog, "Unpacking msgpack element of type: " << a_msgpack_element.type );
        switch ( a_msgpack_element.type )
        {
            case 0: // NULL
                return new param();
                break;
            case 1: // BOOL
                return new param_value( a_msgpack_element.as<bool>() );
                break;
            case 2: // positive int
                return new param_value( a_msgpack_element.as< unsigned >() );
                break;
            case 3: // negative int
                return new param_value( a_msgpack_element.as< int >() );
                break;
            case 4: // float
                return new param_value( a_msgpack_element.as< float >() );
                break;
            case 5: // string
                return new param_value( a_msgpack_element.as< std::string >() );
                break;
            case 6: // array
                {
                    param_array* t_config_array = new param_array();
                    for( unsigned i_element = 0; i_element < a_msgpack_element.via.array.size; ++i_element )
                    {
                        t_config_array->push_back( param_input_msgpack::read_msgpack_element( a_msgpack_element.via.array.ptr[i_element] ) );
                    }
                    return t_config_array;
                    break;
                }
            case 7: // map
                {
                    param_node* t_config_node = new param_node();
                    for( unsigned i_element = 0; i_element < a_msgpack_element.via.array.size; ++i_element )
                    {
                        t_config_node->replace( a_msgpack_element.via.array.ptr[2 * i_element].as<std::string>(),
                                                param_input_msgpack::read_msgpack_element( a_msgpack_element.via.array.ptr[2 * i_element + 1] ) );
                    }
                    return t_config_node;
                    break;
                }
            case 8: // BIN
                MTWARN( mtlog, "Cannot handle 'bin'" );
                return NULL;
                break;
            default:
                MTWARN( mtlog, "Type unrecognized: " << a_msgpack_element.type );
                return NULL;
                break;
        }
        MTWARN( mtlog, "Code should not reach this point" );
        return NULL;
    }

    param_output_msgpack::param_output_msgpack()
    {}

    param_output_msgpack::~param_output_msgpack()
    {}

    bool param_output_msgpack::write_string( const param& /*a_to_write*/, std::string& /*a_string*/, json_writing_style /*a_style*/ )
    {
        return false;
    }

} /* namespace mantis */
