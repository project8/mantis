/*
 * test_mantis_slack.cc
 * Author: N.S. Oblath
 * Date: June 15, 2015
 *
 */

#include "mt_param.hh"
#include "mt_param_json.hh"

#include "urdl/http.hpp"

using namespace mantis;

using std::string;

int main()
{

    param_node t_payload;
    t_payload.add( "token", new param_value( "xxxxx" ) );
    t_payload.add( "channel", new param_value( "#p8_notices" ) );
    t_payload.add( "username", new param_value( "project8" ) );
    t_payload.add( "text", new param_value( "hello slack!" ) );
    string t_payload_str;
    param_output_json::write_string( t_payload, t_payload_str, param_output_json::k_compact );

    urdl::istream t_urdl;
    t_urdl.set_option( urdl::http::request_method( "POST" ) );
    t_urdl.set_option( urdl::http::request_content_type( "application/json" ) );
    t_urdl.set_option( urdl::http::request_content( t_payload_str ) );
    t_urdl.open( "http://slack.com/api/chat.postMessage");

    return 0;
}
