/*
 * mt_run_client.cc
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#include "mt_run_client.hh"

#include "mt_broker.hh"
#include "mt_constants.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_connection.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <algorithm> // for min
#include <string>
#include <unistd.h>
using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "run_client" );

    run_client::run_client( broker* a_broker, const param_node& a_node, const string& a_exe_name ) :
            f_broker( a_broker ),
            f_config( a_node ),
            f_exe_name( a_exe_name ),
            f_canceled( false ),
            f_return( 0 )
    {
    }

    run_client::~run_client()
    {
    }

    void run_client::execute()
    {
        MTINFO( mtlog, "creating request objects..." );

        param_node* t_client_node = new param_node();
        t_client_node->add( "commit", param_value() << TOSTRING(Mantis_GIT_COMMIT) );
        t_client_node->add( "exe", param_value() << f_exe_name );
        t_client_node->add( "version", param_value() << TOSTRING(Mantis_VERSION) );

        param_node* t_request_payload = new param_node();
        t_request_payload->add( "file", f_config.node_at( "file" ) );

        param_node* t_request = new param_node();
        t_request->add( "msgtype", param_value() << T_MANTIS_REQUEST );
        t_request->add( "msgop", param_value() << OP_MANTIS_RUN );
        t_request->add( "target", param_value() << "mantis" );
        t_request->add( "timestamp", param_value() << get_absolute_time_string() );
        t_request->add( "payload", t_request_payload );

        std::string t_request_str;
        if(! param_output_json::write_string( *t_request, t_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "connecting to broker..." );

        connection* t_connection = f_broker->create_connection();
        if( t_connection == NULL )
        {
            MTERROR( mtlog, "Cannot create connection to AMQP broker" );
            f_return = RETURN_ERROR;
            return;
        }

        try
        {
            t_connection->amqp()->DeclareExchange( "requests", AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, true );
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Exchange <request> was not present; aborting.\n(" << e.what() << ")" );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "sending request..." );

        AmqpClient::BasicMessage::ptr_t t_message = AmqpClient::BasicMessage::Create( t_request_str );
        t_message->ContentEncoding( "application/json" );

        try
        {
            t_connection->amqp()->BasicPublish( "requests", "mantis", t_message );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            MTERROR( mtlog, "Message could not be sent: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }

        f_return = RETURN_SUCCESS;

        return;
    }

    void run_client::cancel()
    {
        f_canceled.store( true );
        return;
    }

    int run_client::get_return()
    {
        return f_return;
    }

} /* namespace mantis */
