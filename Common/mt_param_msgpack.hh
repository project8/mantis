/*
 * mt_param_msgpack.hh
 *
 *  Created on: March 27, 2015
 *      Author: bhlaroque
 */

#ifndef MT_PARAM_MSGPACK_HH_
#define MT_PARAM_MSGPACK_HH_

#include "mt_exception.hh"

#include "mt_logger.hh"

#include "mt_param.hh"

#include "document.h"
#include "filestream.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <deque>
#include <map>
#include <sstream>
#include <string>

//#ifdef _WIN32
//MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::basic_string< char, std::char_traits< char >, std::allocator< char > >;
//#endif

namespace mantis
{
    MTLOGGER(mtlog_param_msgpack, "param");

    //***************************************
    //************** INPUT ******************
    //***************************************

    class MANTIS_API param_input_msgpack
    {
        public:
            param_input_msgpack();
            virtual ~param_input_msgpack();

            //static param_node* read_file( const std::string& a_filename );
            static param_node* read_string( const std::string& a_msgpack_str );
            //static param_node* read_document( const rapidjson::Document& a_document );
            //static param* read_value( const rapidjson::Value& a_value );
    };

    //***************************************
    //************** OUTPUT *****************
    //***************************************

    class MANTIS_API param_output_msgpack
    {
        public:
            //typedef rapidjson::Writer< rapidjson::FileStream, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_file_writer;
            //typedef rapidjson::PrettyWriter< rapidjson::FileStream, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_pretty_file_writer;
            //typedef rapidjson::Writer< rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_string_writer;
            //typedef rapidjson::PrettyWriter< rapidjson::StringBuffer, rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<> > rj_pretty_string_writer;

            enum json_writing_style
            {
                k_compact,
                k_pretty
            };

        public:
            param_output_msgpack();
            virtual ~param_output_msgpack();

            static bool write_file( const param& a_to_write, const std::string& a_filename, json_writing_style a_style );
            static bool write_string( const param& a_to_write, std::string& a_string, json_writing_style a_style );
            template< class XWriter >
            static bool write_param( const param& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_null( const param& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_value( const param_value& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_array( const param_array& a_to_write, XWriter* a_writer );
            template< class XWriter >
            static bool write_param_node( const param_node& a_to_write, XWriter* a_writer );

    };

    template< class XWriter >
    bool param_output_msgpack::write_param( const param& a_to_write, XWriter* a_writer )
    {
        if( a_to_write.is_null() )
        {
            return param_output_msgpack::write_param_null( a_to_write, a_writer );
        }
        if( a_to_write.is_value() )
        {
            return param_output_msgpack::write_param_value( a_to_write.as_value(), a_writer );
        }
        if( a_to_write.is_array() )
        {
            return param_output_msgpack::write_param_array( a_to_write.as_array(), a_writer );
        }
        if( a_to_write.is_node() )
        {
            return param_output_msgpack::write_param_node( a_to_write.as_node(), a_writer );
        }
        MTWARN( mtlog_param_msgpack, "parameter not written: <" << a_to_write << ">" );
        return false;
    }
    template< class XWriter >
    bool param_output_msgpack::write_param_null( const param& /*a_to_write*/, XWriter* a_writer )
    {
        //MTWARN( mtlog_param_msgpack, "writing null" );
        a_writer->Null();
        return true;
    }
    template< class XWriter >
    bool param_output_msgpack::write_param_value( const param_value& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_param_msgpack, "writing value" );
        if( a_to_write.is_string() )
        {
            a_writer->String( a_to_write.as_string().c_str() );
            //MTWARN( mtlog_param_msgpack, "writing string to msgpack: " << a_to_write.as_string() );
        }
        else if( a_to_write.is_bool() )
        {
            a_writer->Bool( a_to_write.as_bool() );
            //MTWARN( mtlog_param_msgpack, "writing bool to msgpack: " << a_to_write.as_bool() );
        }
        else if( a_to_write.is_int() )
        {
            a_writer->Int64( a_to_write.as_int() );
            //MTWARN( mtlog_param_msgpack, "writing int to msgpack: " << a_to_write.as_int() );
        }
        else if( a_to_write.is_uint() )
        {
            a_writer->Uint64( a_to_write.as_uint() );
            //MTWARN( mtlog_param_msgpack, "writing uint to msgpack: " << a_to_write.as_uint() );
        }
        else if( a_to_write.is_double() )
        {
            a_writer->Double( a_to_write.as_double() );
            //MTWARN( mtlog_param_msgpack, "writing double to msgpack: " << a_to_write.as_double() );
        }
        return true;
    }
    template< class XWriter >
    bool param_output_msgpack::write_param_array( const param_array& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_param_msgpack, "writing array" );
        a_writer->StartArray();
        for( param_array::const_iterator it = a_to_write.begin(); it != a_to_write.end(); ++it )
        {
            if( ! param_output_msgpack::write_param( *(*it), a_writer ) )
            {
                MTERROR( mtlog_param_msgpack, "Error while writing parameter array" );
                return false;
            }
        }
        a_writer->EndArray();
        return true;
    }
    template< class XWriter >
    bool param_output_msgpack::write_param_node( const param_node& a_to_write, XWriter* a_writer )
    {
        //MTWARN( mtlog_param_msgpack, "writing node" );
        a_writer->StartObject();
        for( param_node::const_iterator it = a_to_write.begin(); it != a_to_write.end(); ++it )
        {
            a_writer->String( it->first.c_str() );
            if( ! param_output_msgpack::write_param( *(it->second), a_writer ) )
            {
                MTERROR( mtlog_param_msgpack, "Error while writing parameter node" );
                return false;
            }
        }
        a_writer->EndObject();
        return true;
    }


} /* namespace mantis */

#endif /* MT_PARAM_MSGPACK_HH_ */
