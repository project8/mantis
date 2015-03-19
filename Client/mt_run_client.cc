/*
 * mt_run_client.cc
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_run_client.hh"

#include "mt_broker.hh"
#include "mt_connection.hh"
#include "mt_constants.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <algorithm> // for min
#include <string>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "run_client" );

    run_client::run_client( const param_node& a_node, const string& a_exe_name ) :
            //callable(),
            f_config( a_node ),
            f_exe_name( a_exe_name ),
            //f_canceled( false ),
            f_return( 0 )
    {
    }

    run_client::~run_client()
    {
    }

    void run_client::execute()
    {
        MTINFO( mtlog, "Connecting to AMQP broker" );

        const param_node* t_broker_node = f_config.node_at( "amqp" );

        broker t_broker( t_broker_node->get_value( "broker" ),
                         t_broker_node->get_value< unsigned >( "broker-port" ) );

        connection* t_connection = t_broker.create_connection();
        if( t_connection == NULL )
        {
            MTERROR( mtlog, "Cannot create connection to AMQP broker" );
            f_return = RETURN_ERROR;
            return;
        }

        std::string t_exchange;
        try
        {
            t_exchange = t_broker_node->get_value( "exchange" );
            t_connection->amqp()->DeclareExchange( t_exchange, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT, true );
        }
        catch( std::exception& e )
        {
            delete t_connection;
            MTERROR( mtlog, "Unable to declare exchange <" << t_exchange << ">; aborting.\n(" << e.what() << ")" );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "Creating request" );

        std::string t_request_type( f_config.get_value( "request", "" ) );

        std::string t_reply_to, t_correlation_id, t_request_str, t_consumer_tag;
        param_node t_save_node;
        // t_consumer_tag will be used to determine whether we should wait for a reply message.
        // if it's empty, we will not wait
        if( t_request_type == "run" )
        {
            if( ! do_run_request( t_request_str ) )
            {
                MTERROR( mtlog, "There was an error while processing the run request" );
                f_return = RETURN_ERROR;
                return;
            }
        }
        else if( t_request_type == "get" )
        {
            if( ! do_get_request( t_request_str, t_connection, t_consumer_tag, t_reply_to, t_save_node ) )
            {
                MTERROR( mtlog, "There was an error while processing the get request" );
                f_return = RETURN_ERROR;
                return;
            }
        }
        else if( t_request_type == "set" )
        {
            if( ! do_set_request( t_request_str, t_connection, t_consumer_tag, t_reply_to ) )
            {
                MTERROR( mtlog, "There was an error while processing the set request" );
                f_return = RETURN_ERROR;
                return;
            }
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
            t_connection->amqp()->BasicPublish( t_exchange, t_broker_node->get_value( "route" ), t_message );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            delete t_connection;
            MTERROR( mtlog, "Message could not be sent: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }
        catch( std::exception& e )
        {
            delete t_connection;
            MTERROR( mtlog, "Error publishing to queue: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }

        if( ! t_consumer_tag.empty() )  // then we should wait for a reply
        {
            MTINFO( mtlog, "Waiting for a reply from the server; use ctrl-c to cancel" );

            // blocking call to wait for incoming message
            AmqpClient::Envelope::ptr_t t_envelope = t_connection->amqp()->BasicConsumeMessage( t_consumer_tag );

            MTINFO( mtlog, "Response received" );

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

            // optionally save "master-config" from the response
            if( t_save_node.size() != 0 )
            {
                if( t_save_node.has( "json" ) )
                {
                    string t_save_filename( t_save_node.get_value( "json" ) );
                    const param_node* t_master_config_node = t_msg_node->node_at( "payload" )->node_at( "master-config" );
                    if( t_master_config_node == NULL )
                    {
                        MTERROR( mtlog, "Node \"master-config\" is not present to save" );
                    }
                    else
                    {
                        param_output_json::write_file( *t_master_config_node, t_save_filename, param_output_json::k_pretty );
                    }
                }
                else
                {
                    MTERROR( mtlog, "Save instruction did not contain a valid file type");
                }

            }

            delete t_msg_node;
        }

        delete t_connection;

        f_return = RETURN_SUCCESS;

        return;
    }
    /*
    void run_client::cancel()
    {
        f_canceled.store( true );
        return;
    }
    */
    int run_client::get_return()
    {
        return f_return;
    }

    bool run_client::do_run_request( std::string& a_request_str )
    {
        const param_node* t_file_node = f_config.node_at( "file" );
        if( t_file_node == NULL )
        {
            MTERROR( mtlog, "No file configuration is present" );
            return false;
        }

        param_node* t_client_node = new param_node();
        t_client_node->add( "commit", param_value() << TOSTRING(Mantis_GIT_COMMIT) );
        t_client_node->add( "exe", param_value() << f_exe_name );
        t_client_node->add( "version", param_value() << TOSTRING(Mantis_VERSION) );

        param_node* t_request_payload = new param_node();
        t_request_payload->add( "file", *t_file_node ); // make a copy of t_file_node
        t_request_payload->add( "client", t_client_node ); // use t_client_node as is

        param_node t_request;
        t_request.add( "msgtype", param_value() << T_REQUEST );
        t_request.add( "msgop", param_value() << OP_RUN );
        //t_request.add( "target", param_value() << "mantis" );  // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value() << get_absolute_time_string() );
        t_request.add( "payload", t_request_payload ); // use t_request_node as is

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        return true;
    }

    bool run_client::do_get_request( std::string& a_request_str, connection* a_connection, std::string& a_consumer_tag, std::string& a_reply_to, param_node& a_save_node )
    {
        string t_query_type = f_config.get_value( "get", "" );
        if( t_query_type.empty() )
        {
            MTERROR( mtlog, "Get type was not specified" );
            return false;
        }

        param_node t_payload_node;
        t_payload_node.add( "get", param_value() << t_query_type );

        param_node t_request;
        t_request.add( "msgtype", param_value() << T_REQUEST );
        t_request.add( "msgop", param_value() << OP_GET );
        //t_request.add( "target", param_value() << "mantis" );  // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value() << get_absolute_time_string() );
        t_request.add( "payload", t_payload_node );

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        a_reply_to = a_connection->amqp()->DeclareQueue( "" );
        a_consumer_tag = a_connection->amqp()->BasicConsume( a_reply_to );
        MTDEBUG( mtlog, "Consumer tag for reply: " << a_consumer_tag );

        // check for whether we'll be saving the result
        a_save_node.clear();
        if( f_config.has( "save" ) )
        {
            a_save_node = *(f_config.node_at( "save" ));
        }

        return true;
    }

    bool run_client::do_set_request( std::string& a_request_str, connection* a_connection, std::string& a_consumer_tag, std::string& a_reply_to )
    {
        param_node t_payload_node;

        string t_instruction;
        if( f_config.has( "set" ) )
        {
            t_instruction = "set";
        }
        else if( f_config.has( "load" ) )
        {
            t_instruction = "load";
        }
        else if( f_config.has( "add" ) )
        {
            t_instruction = "add";
        }
        else if( f_config.has( "remove" ) )
        {
            t_instruction = "remove";
        }
        else
        {
            MTERROR( mtlog, "No valid set instruction was specified" );
            return false;
        }

        param_node* t_instruction_node = new param_node( *f_config.node_at( t_instruction ));
        if( t_instruction_node == NULL )
        {
            delete t_instruction_node;
            MTERROR( mtlog, "Instruction for <" << t_instruction << "> was not specified" );
            return false;
        }

        // for the load instruction, the instruction node should be replaced by the contents of the file specified
        if( t_instruction == "load" )
        {
            if( t_instruction_node->has( "json" ) )
            {
                string t_load_filename( t_instruction_node->get_value( "json" ) );
                delete t_instruction_node;
                t_instruction_node = param_input_json::read_file( t_load_filename );
                if( t_instruction_node == NULL )
                {
                    MTERROR( mtlog, "Unable to read JSON file <" << t_load_filename << ">" );
                    return false;
                }
            }
            else
            {
                delete t_instruction_node;
                MTERROR( mtlog, "Load instruction did not contain a valid file type");
                return false;
            }
        }

        t_payload_node.add( "action", param_value() << t_instruction );
        t_payload_node.add( t_instruction, t_instruction_node ); // use t_instruction_node itself, so it doesn't have to be deleted

        param_node t_request;
        t_request.add( "msgtype", param_value() << T_REQUEST );
        t_request.add( "msgop", param_value() << OP_SET );
        //t_request.add( "target", param_value() << "mantis" ); // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value() << get_absolute_time_string() );
        t_request.add( "payload", t_payload_node );

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        a_reply_to = a_connection->amqp()->DeclareQueue( "" );
        a_consumer_tag = a_connection->amqp()->BasicConsume( a_reply_to );
        MTDEBUG( mtlog, "Consumer tag for reply: " << a_consumer_tag );

        return true;
    }


} /* namespace mantis */
