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
#include "mt_param_json.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <algorithm> // for min
#include <string>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "run_client" );

    run_client::run_client( const param_node& a_node, const string& a_exe_name, const string& a_exchange ) :
            //callable(),
            f_config( a_node ),
            f_exe_name( a_exe_name ),
            f_exchange( a_exchange ),
            //f_canceled( false ),
            f_return( 0 )
    {
    }

    run_client::~run_client()
    {
    }

    void run_client::execute()
    {
        broker* t_broker = broker::get_instance();

        MTINFO( mtlog, "Creating request" );

        std::string t_request_type( f_config.get_value( "do", "" ) );
        f_config.erase( "do" );

        std::string t_routing_key( f_config.get_value( "dest", "mantis" ) );
        f_config.erase( "dest" );

        param_node t_save_node;
        if( f_config.has( "save" ) )
        {
            t_save_node = *(f_config.node_at( "save" ));
        }
        f_config.erase( "save" );

        // now all that remains in f_config should be values to pass to the server as arguments to the request

        std::string t_request_str;
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
            if( ! do_get_request( t_request_str ) )
            {
                MTERROR( mtlog, "There was an error while processing the get request" );
                f_return = RETURN_ERROR;
                return;
            }
        }
        else if( t_request_type == "set" )
        {
            if( ! do_set_request( t_request_str ) )
            {
                MTERROR( mtlog, "There was an error while processing the set request" );
                f_return = RETURN_ERROR;
                return;
            }
        }
        else if( t_request_type == "cmd" )
        {
            if( ! do_cmd_request( t_request_str ) )
            {
                MTERROR( mtlog, "There was an error while processing the cmd request" );
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


        // strings for passing to the various do_[type]_request functions
        std::string t_reply_to = broker::get_instance()->get_connection().amqp()->DeclareQueue( "" );
        std::string t_consumer_tag = broker::get_instance()->get_connection().  amqp()->BasicConsume( t_reply_to );
        MTDEBUG( mtlog, "Consumer tag for reply: " << t_consumer_tag );

        MTINFO( mtlog, "Sending request with routing key <" << t_routing_key << ">" );

        AmqpClient::BasicMessage::ptr_t t_message = AmqpClient::BasicMessage::Create( t_request_str );
        t_message->ContentEncoding( "application/json" );
        std::string t_correlation_id;
        t_message->CorrelationId( t_correlation_id ); // currently always ""
        t_message->ReplyTo( t_reply_to );

        try
        {
            t_broker->get_connection().amqp()->BasicPublish( f_exchange, t_routing_key, t_message, true, true );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            MTERROR( mtlog, "Message could not be sent: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Error publishing to queue: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }

        if( ! t_consumer_tag.empty() )  // then we should wait for a reply
        {
            MTINFO( mtlog, "Waiting for a reply from the server; use ctrl-c to cancel" );

            // blocking call to wait for incoming message
            AmqpClient::Envelope::ptr_t t_envelope = t_broker->get_connection().amqp()->BasicConsumeMessage( t_consumer_tag );

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
                    const param_node* t_master_config_node = t_msg_node->node_at( "payload" )->node_at( "content" );
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
        if( ! f_config.has( "file" ) )
        {
            MTERROR( mtlog, "The filename to be saved must be specified with the \"file\" option" );
            return false;
        }

        param_node* t_client_node = new param_node();
        t_client_node->add( "commit", param_value( TOSTRING(Mantis_GIT_COMMIT) ) );
        t_client_node->add( "exe", param_value( f_exe_name ) );
        t_client_node->add( "version", param_value( TOSTRING(Mantis_VERSION) ) );

        param_node* t_payload_node = new param_node( f_config ); // copy of f_config, which should consist of only the request arguments
        t_payload_node->add( "client", t_client_node ); // use t_client_node as is
        t_payload_node->add( "file", *f_config.at( "file ") ); // copy the file node
        if( f_config.has( "description" ) ) t_payload_node->add( "description", *f_config.at( "description" ) ); // (optional) copy the description node

        param_node t_request;
        t_request.add( "msgtype", param_value( T_REQUEST ) );
        t_request.add( "msgop", param_value( OP_RUN ) );
        //t_request.add( "target", param_value( "mantis" ) );  // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value( get_absolute_time_string() ) );
        t_request.add( "payload", t_payload_node ); // use t_payload_node as is

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        return true;
    }

    bool run_client::do_get_request( std::string& a_request_str )
    {
        param_node* t_payload_node = new param_node();

        if( f_config.has( "value" ) )
        {
            param_array* t_values_array = new param_array();
            t_values_array->push_back( f_config.remove( "value" ) );
            t_payload_node->add( "values", t_values_array );
        }

        param_node t_request;
        t_request.add( "msgtype", param_value( T_REQUEST ) );
        t_request.add( "msgop", param_value( OP_GET ) );
        //t_request.add( "target", param_value( "mantis" ) );  // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value( get_absolute_time_string() ) );
        t_request.add( "payload", t_payload_node ); // use t_payload_node as is

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        return true;
    }

    bool run_client::do_set_request( std::string& a_request_str )
    {
        if( ! f_config.has( "value" ) )
        {
            MTERROR( mtlog, "No \"value\" option given" );
            return false;
        }

        param_array* t_values_array = new param_array();
        t_values_array->push_back( f_config.remove( "value" ) );

        param_node* t_payload_node = new param_node();
        t_payload_node->add( "values", t_values_array );

        param_node t_request;
        t_request.add( "msgtype", param_value( T_REQUEST ) );
        t_request.add( "msgop", param_value( OP_SET ) );
        //t_request.add( "target", param_value( "mantis" ) ); // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value( get_absolute_time_string() ) );
        t_request.add( "payload", t_payload_node ); // use t_payload_node as is

        MTDEBUG( mtlog, "Sending message:\n" << t_request );

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        return true;
    }

    bool run_client::do_cmd_request( std::string& a_request_str )
    {
        param_node* t_payload_node = new param_node();

        // for the load instruction, the instruction node should be replaced by the contents of the file specified
        if( f_config.has( "load" ) )
        {
            if( ! f_config.node_at( "load" )->has( "json" ) )
            {
                MTERROR( mtlog, "Load instruction did not contain a valid file type");
                delete t_payload_node;
                return false;
            }

            string t_load_filename( f_config.node_at( "load" )->get_value( "json" ) );
            param_node* t_node_from_file = param_input_json::read_file( t_load_filename );
            if( t_node_from_file == NULL )
            {
                MTERROR( mtlog, "Unable to read JSON file <" << t_load_filename << ">" );
                delete t_payload_node;
                return false;
            }

            t_payload_node->merge( *t_node_from_file );
            f_config.erase( "load" );
        }

        // at this point, all that remains in f_config should be other options that we want to add to the payload node
        t_payload_node->merge( f_config ); // copy f_config

        param_node t_request;
        t_request.add( "msgtype", param_value( T_REQUEST ) );
        t_request.add( "msgop", param_value( OP_CMD ) );
        //t_request.add( "target", param_value( "mantis" ) ); // use of the target is now deprecated (3/12/15)
        t_request.add( "timestamp", param_value( get_absolute_time_string() ) );
        t_request.add( "payload", t_payload_node ); // use t_payload_node as is

        MTDEBUG( mtlog, "Sending message:\n" << t_request );

        if(! param_output_json::write_string( t_request, a_request_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert request to string" );
            return false;
        }

        return true;
    }


} /* namespace mantis */
