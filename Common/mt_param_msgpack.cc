/*
 * mt_param_msgpack.cc
 *
 *  Created on: March 27, 2015
 *      Author: bhlaroque
 */

#define MANTIS_API_EXPORTS

#include "mt_param_msgpack.hh"

#include "mt_logger.hh"

#include <msgpack.hpp>

#include <sstream>
using std::string;
using std::stringstream;


#include <cstdio>
using std::string;


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
        //rapidjson::Document t_config_doc;
        msgpack::type::map< Stream, Stream > t_config_doc;
        msgpack::unpacked result;
        msgpack::unpack(result, a_msgpack_string, a_msgpack_string.size());
        /*if( t_config_doc.Parse<0>( a_msgpack_string.c_str() ).HasParseError() )
        {
            MTERROR( mtlog, "error parsing string:\n" << t_config_doc.GetParseError() );
            return NULL;
        }*/
        return NULL;//param_input_msgpack::read_document( t_config_doc );
    }

    param* param_input_msgpack::read_value( const rapidjson::Value& a_value )
    {
        if( a_value.IsNull() )
        {
            return new param();
        }
        if( a_value.IsObject() )
        {
            param_node* t_config_object = new param_node();
            for( rapidjson::Value::ConstMemberIterator jsonIt = a_value.MemberBegin();
                    jsonIt != a_value.MemberEnd();
                    ++jsonIt)
            {
                t_config_object->replace( jsonIt->name.GetString(), param_input_msgpack::read_value( jsonIt->value ) );
            }
            return t_config_object;
        }
        if( a_value.IsArray() )
        {
            param_array* t_config_array = new param_array();
            for( rapidjson::Value::ConstValueIterator jsonIt = a_value.Begin();
                    jsonIt != a_value.End();
                    ++jsonIt)
            {
                t_config_array->push_back( param_input_msgpack::read_value( *jsonIt ) );
            }
            return t_config_array;
        }
        if( a_value.IsString() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetString();
            return t_config_value;
        }
        if( a_value.IsBool() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetBool();
            return t_config_value;
        }
        if( a_value.IsInt() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetInt();
            return t_config_value;
        }
        if( a_value.IsUint() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetUint();
            return t_config_value;
        }
        if( a_value.IsInt64() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetInt64();
            return t_config_value;
        }
        if( a_value.IsUint64() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetUint64();
            return t_config_value;
        }
        if( a_value.IsDouble() )
        {
            param_value* t_config_value = new param_value();
            (*t_config_value) << a_value.GetDouble();
            return t_config_value;
        }
        MTWARN( mtlog, "(config_reader_msgpack) unknown type; returning null value" );
        return new param();
    }


    param_output_msgpack::param_output_msgpack()
    {}

    param_output_msgpack::~param_output_msgpack()
    {}

    bool param_output_msgpack::write_file( const param& a_to_write, const std::string& a_filename, json_writing_style a_style )
    {
        if( a_filename.empty() )
        {
            MTERROR( mtlog, "Filename cannot be an empty string" );
            return false;
        }

        FILE* file = fopen( a_filename.c_str(), "w" );
        if( file == NULL )
        {
            MTERROR( mtlog, "Unable to open file: " << a_filename );
            return false;
        }

        rapidjson::FileStream t_filestream( file );

        bool t_result = false;
        if( a_style == k_compact )
        {
            rj_file_writer t_writer( t_filestream );
            t_result = param_output_msgpack::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_file_writer t_writer( t_filestream );
            t_result = param_output_msgpack::write_param( a_to_write, &t_writer );
        }

        if (! t_result )
        {
            MTERROR( mtlog, "Error while writing file" );
            return false;
        }

        return true;
    }
    bool param_output_msgpack::write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style )
    {
        rapidjson::StringBuffer t_str_buff;

        bool t_result = false;
        if( a_style == k_compact )
        {
            rj_string_writer t_writer( t_str_buff );
            t_result = param_output_msgpack::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_string_writer t_writer( t_str_buff );
            t_result = param_output_msgpack::write_param( a_to_write, &t_writer );
        }

        if (! t_result )
        {
            MTERROR( mtlog, "Error while writing string" );
            return false;
        }

        a_string.assign( t_str_buff.GetString() );

        return true;
    }

} /* namespace mantis */
