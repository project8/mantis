/*
 * test_mantis_slack.cc
 * Author: N.S. Oblath
 * Date: June 15, 2015
 *
 */

#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_param_json.hh"

//#include "urdl/http.hpp"
//#include "urdl/istream.hpp"

#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/SSLManager.h>
//#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

using namespace Poco::Net;
using namespace Poco;

using namespace mantis;

using std::string;

MTLOGGER( mtlog, "test_slack" )

int main()
{
    param_node t_payload;
    t_payload.add( "token", new param_value( "xoxb-6285513396-KSmuYsm54LNfhK9R0xejtN8b" ) );
    t_payload.add( "channel", new param_value( "#p8_notices" ) );
    t_payload.add( "username", new param_value( "project8" ) );
    t_payload.add( "text", new param_value( "hello slack!" ) );
    string t_payload_str;
    param_output_json::write_string( t_payload, t_payload_str, param_output_json::k_compact );

    MTINFO( mtlog, "Payload: " << t_payload_str );

    try
      {
        /*
        SharedPtr<PrivateKeyPassphraseHandler> pConsoleHandler = new KeyConsoleHandler( false );
        SharedPtr<InvalidCertificateHandler> pInvalidCertHandler = new ConsoleCertificateHandler( true );
        Context::Ptr pContext = new Context(Context::CLIENT_USE, "", Context::VERIFY_RELAXED, 9, false, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        SSLManager::instance().initializeClient(pConsoleHandler, pInvalidCertHandler, pContext);
        */

        // prepare session
        URI t_uri( "https://slack.com" );
        //HTTPSClientSession t_session( t_uri.getHost(), t_uri.getPort() );
        HTTPClientSession t_session( t_uri.getHost(), t_uri.getPort() );
        t_session.setKeepAlive( true );

        // prepare path
        //string t_path( "/api/chat.postMessage" );
        string t_path( "/api/api.test" );
        MTINFO( mtlog, "URL: " << t_path );

        // send request
        HTTPRequest t_req( HTTPRequest::HTTP_POST, t_path, HTTPMessage::HTTP_1_1 );
        t_req.setContentType( "application/json" );
        t_req.setKeepAlive( true );
        //t_req.add( "payload", t_payload_str );

        t_req.setContentLength( t_payload_str.length() );

        std::ostream& t_req_ostr = t_session.sendRequest( t_req );
        t_req_ostr << t_payload_str;

        t_req.write( std::cout );

        // get response
        HTTPResponse t_res;
        std::istream &t_resp_istr = t_session.receiveResponse(t_res);
        MTINFO( mtlog, "Response: " << t_resp_istr );
        //StreamCopier::copyStream(is, cout);
      }
      catch (Exception &e)
      {
        MTERROR( mtlog, e.displayText() );
        return -1;
      }


    /*
    urdl::istream t_urdl;
    t_urdl.set_option( urdl::http::request_method( "POST" ) );
    t_urdl.set_option( urdl::http::request_content_type( "application/json" ) );
    t_urdl.set_option( urdl::http::request_content( t_payload_str ) );
    t_urdl.open( "http://slack.com/api/chat.postMessage");
    */

    return 0;
}
