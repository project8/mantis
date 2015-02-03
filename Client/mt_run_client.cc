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

    run_client::run_client( const param_node& a_node, const string& a_exe_name ) :
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
        MTINFO( mtlog, "Connecting to broker" );

        const param_node* t_broker_node = f_config.node_at( "broker" );

        broker t_broker( t_broker_node->get_value( "addr" ),
                         t_broker_node->get_value< unsigned >( "port" ) );

        connection* t_connection = t_broker.create_connection();
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
            delete t_connection;
            MTERROR( mtlog, "Exchange <request> was not present; aborting.\n(" << e.what() << ")" );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "Creating request" );

        std::string t_request_type( f_config.get_value( "request", "" ) );

        std::string t_reply_to, t_correlation_id, t_request_str, t_consumer_tag;
        // t_consumer_tag will be used to determine whether we should wait for a reply message.
        // if it's empty, we will not wait
        if( t_request_type == "run" )
        {
            const param_node* t_file_node = f_config.node_at( "file" );
            if( t_file_node == NULL )
            {
                MTERROR( mtlog, "No file configuration is present" );
                f_return = RETURN_ERROR;
                return;
            }

            param_node* t_client_node = new param_node();
            t_client_node->add( "commit", param_value() << TOSTRING(Mantis_GIT_COMMIT) );
            t_client_node->add( "exe", param_value() << f_exe_name );
            t_client_node->add( "version", param_value() << TOSTRING(Mantis_VERSION) );

            param_node* t_request_payload = new param_node();
            t_request_payload->add( "file", *t_file_node ); // make a copy of t_file_node
            t_request_payload->add( "client", t_client_node ); // use t_client_node as is

            param_node t_request;
            t_request.add( "msgtype", param_value() << T_MANTIS_REQUEST );
            t_request.add( "msgop", param_value() << OP_MANTIS_RUN );
            t_request.add( "target", param_value() << "mantis" );
            t_request.add( "timestamp", param_value() << get_absolute_time_string() );
            t_request.add( "payload", t_request_payload ); // use t_request_node as is

            if(! param_output_json::write_string( t_request, t_request_str, param_output_json::k_compact ) )
            {
                MTERROR( mtlog, "Could not convert request to string" );
                f_return = RETURN_ERROR;
                return;
            }
        }
        else if( t_request_type == "query" )
        {
            const param_node* t_query_node = f_config.node_at( "query" );
            if( t_query_node == NULL )
            {
                MTERROR( mtlog, "Particular query was not specified" );
                f_return = RETURN_ERROR;
                return;
            }

            param_node t_request;
            t_request.add( "msgtype", param_value() << T_MANTIS_REQUEST );
            t_request.add( "msgop", param_value() << OP_MANTIS_QUERY );
            t_request.add( "target", param_value() << "mantis" );
            t_request.add( "timestamp", param_value() << get_absolute_time_string() );
            t_request.add( "payload", *t_query_node ); // make a copy of t_query_node

            if(! param_output_json::write_string( t_request, t_request_str, param_output_json::k_compact ) )
            {
                MTERROR( mtlog, "Could not convert request to string" );
                f_return = RETURN_ERROR;
                return;
            }

            t_reply_to = t_connection->amqp()->DeclareQueue( "" );
            std::string t_consumer_tag = t_connection->amqp()->BasicConsume( t_reply_to );
            MTDEBUG( mtlog, "consumer tag for reply: " << t_consumer_tag );

        }
        else if( t_request_type == "config" )
        {
            MTERROR( mtlog, "Config requests not yet supported" );
            f_return = RETURN_ERROR;
            return;
        }
        else
        {
            MTERROR( mtlog, "Unknown or missing request type: " << t_request_type );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "Sending request" );

        AmqpClient::BasicMessage::ptr_t t_message = AmqpClient::BasicMessage::Create( t_request_str );
        t_message->ContentEncoding( "application/json" );
        t_message->CorrelationId( t_correlation_id ); // currently always ""
        t_message->ReplyTo( t_reply_to );

        try
        {
            t_connection->amqp()->BasicPublish( "requests", "mantis", t_message );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            delete t_connection;
            MTERROR( mtlog, "Message could not be sent: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }

        if( ! t_consumer_tag.empty() )  // then we should wait for a reply
        {
            // blocking call to wait for incoming message
            AmqpClient::Envelope::ptr_t t_envelope = t_connection->amqp()->BasicConsumeMessage( t_consumer_tag );

            param_node* t_msg_node = NULL;
            if( t_envelope->Message()->ContentEncoding() == "application/json" )
            {
                t_msg_node = param_input_json::read_string( t_envelope->Message()->Body() );
            }
            else
            {
                MTERROR( mtlog, "Unable to parse message with content type <" << t_envelope->Message()->ContentEncoding() << ">" );
            }

            MTINFO( mtlog, "Response from Mantis:\n" << *t_msg_node->node_at( "payload" ) );
            delete t_msg_node;
        }

        delete t_connection;

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
