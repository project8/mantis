/*
 * mt_param_json.cc
 *
 *  Created on: March 27, 2015
 *      Author: nsoblath, bhlaroque
 */

#define MANTIS_API_EXPORTS

#include "mt_param_json.hh"

#include "mt_logger.hh"

#include <sstream>
using std::string;
using std::stringstream;


#include <cstdio>


namespace mantis
{
    MTLOGGER( mtlog, "param" );

    param_input_json::param_input_json()
    {
    }
    param_input_json::~param_input_json()
    {
    }

    param_node* param_input_json::read_file( const std::string& a_filename )
    {
        FILE* t_config_file = fopen( a_filename.c_str(), "r" );
        if( t_config_file == NULL )
        {
            MTERROR( mtlog, "file <" << a_filename << "> did not open" );
            return NULL;
        }
        rapidjson::FileStream t_file_stream( t_config_file );

        rapidjson::Document t_config_doc;
        if( t_config_doc.ParseStream<0>( t_file_stream ).HasParseError() )
        {
            unsigned errorPos = t_config_doc.GetErrorOffset();
            rewind( t_config_file );
            unsigned iChar, newlineCount = 1, lastNewlinePos = 0;
            int thisChar;
            for( iChar = 0; iChar != errorPos; ++iChar )
            {
                thisChar = fgetc( t_config_file );
                if( thisChar == EOF )
                {
                    break;
                }
                if( thisChar == '\n' || thisChar == '\r' )
                {
                    newlineCount++;
                    lastNewlinePos = iChar + 1;
                }
            }
            if( iChar == errorPos )
            {
                MTERROR( mtlog, "error parsing config file :\n" <<
                        '\t' << t_config_doc.GetParseError() << '\n' <<
                        "\tThe error was reported at line " << newlineCount << ", character " << errorPos - lastNewlinePos );
            }
            else
            {
                MTERROR( mtlog, "error parsing config file :\n" <<
                        '\t' << t_config_doc.GetParseError() <<
                        "\tend of file reached before error location was found" );
            }
            fclose( t_config_file );
            return NULL;
        }
        fclose( t_config_file );

        return param_input_json::read_document( t_config_doc );
    }

    param_node* param_input_json::read_string( const std::string& a_json_string )
    {
        rapidjson::Document t_config_doc;
        if( t_config_doc.Parse<0>( a_json_string.c_str() ).HasParseError() )
        {
            MTERROR( mtlog, "error parsing string:\n" << t_config_doc.GetParseError() );
            return NULL;
        }
        return param_input_json::read_document( t_config_doc );
    }

    param_node* param_input_json::read_document( const rapidjson::Document& a_doc )
    {
        param_node* t_config = new param_node();
        for( rapidjson::Value::ConstMemberIterator jsonIt = a_doc.MemberBegin();
                jsonIt != a_doc.MemberEnd();
                ++jsonIt)
        {
            t_config->replace( jsonIt->name.GetString(), param_input_json::read_value( jsonIt->value ) );
        }
        return t_config;
    }

    param* param_input_json::read_value( const rapidjson::Value& a_value )
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
                t_config_object->replace( jsonIt->name.GetString(), param_input_json::read_value( jsonIt->value ) );
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
                t_config_array->push_back( param_input_json::read_value( *jsonIt ) );
            }
            return t_config_array;
        }
        if( a_value.IsString() )
        {
            //MTWARN( mtlog, "reading string from json: " << a_value.GetString() );
            return new param_value( a_value.GetString() );
        }
        if( a_value.IsBool() )
        {
            //MTWARN( mtlog, "reading bool from json: " << a_value.GetBool() );
            return new param_value( a_value.GetBool() );
        }
        if( a_value.IsInt() )
        {
            //MTWARN( mtlog, "reading int from json: " << a_value.GetInt() );
            return new param_value( a_value.GetInt() );
        }
        if( a_value.IsUint() )
        {
            //MTWARN( mtlog, "reading uint from json: " << a_value.GetUint() );
            return new param_value( a_value.GetUint() );
        }
        if( a_value.IsInt64() )
        {
            //MTWARN( mtlog, "reading int64 from json: " << a_value.GetInt64() );
            return new param_value( a_value.GetInt64() );
        }
        if( a_value.IsUint64() )
        {
            //MTWARN( mtlog, "reading uint64 from json: " << a_value.GetUint64() );
            return new param_value( a_value.GetUint64() );
        }
        if( a_value.IsDouble() )
        {
            //MTWARN( mtlog, "reading double from json: " << a_value.GetDouble() );
            return new param_value( a_value.GetDouble() );
        }
        MTWARN( mtlog, "(config_reader_json) unknown type; returning null value" );
        return new param();
    }


    param_output_json::param_output_json()
    {}

    param_output_json::~param_output_json()
    {}

    bool param_output_json::write_file( const param& a_to_write, const std::string& a_filename, json_writing_style a_style )
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
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_file_writer t_writer( t_filestream );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }

        if (! t_result )
        {
            MTERROR( mtlog, "Error while writing file" );
            return false;
        }

        return true;
    }
    bool param_output_json::write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style )
    {
        rapidjson::StringBuffer t_str_buff;

        bool t_result = false;
        if( a_style == k_compact )
        {
            rj_string_writer t_writer( t_str_buff );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
        }
        else
        {
            rj_pretty_string_writer t_writer( t_str_buff );
            t_result = param_output_json::write_param( a_to_write, &t_writer );
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
