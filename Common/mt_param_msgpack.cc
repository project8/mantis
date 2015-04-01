/*
 * mt_param_msgpack.cc
 *
 *  Created on: March 27, 2015
 *      Author: bhlaroque
 */

#define MANTIS_API_EXPORTS

#include "mt_param_msgpack.hh"

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

    param_node* param_input_msgpack::read_string( const std::string& a_json_string )
    {
        rapidjson::Document t_config_doc;
        if( t_config_doc.Parse<0>( a_json_string.c_str() ).HasParseError() )
        {
            MTERROR( mtlog, "error parsing string:\n" << t_config_doc.GetParseError() );
            return NULL;
        }
        return NULL;//param_input_msgpack::read_document( t_config_doc );
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
