/*
 * mt_run_client.cc
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_run_client.hh"

#include "mt_constants.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_message.hh"
#include "mt_param_json.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <algorithm> // for min
#include <string>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "run_client" );

    run_client::run_client( const param_node& a_node, const string& a_exchange, amqp_channel_ptr a_channel ) :
            //callable(),
            f_config( a_node ),
            f_exchange( a_exchange ),
            f_channel( a_channel ),
            //f_canceled( false ),
            f_return( 0 )
    {
    }

    run_client::~run_client()
    {
    }

    void run_client::execute()
    {
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

        msg_request* t_request = NULL;
        if( t_request_type == "run" )
        {
            t_request = create_run_request( t_routing_key );
        }
        else if( t_request_type == "get" )
        {
            t_request = create_get_request( t_routing_key );
        }
        else if( t_request_type == "set" )
        {
            t_request = create_set_request( t_routing_key );
        }
        else if( t_request_type == "cmd" )
        {
            t_request = create_cmd_request( t_routing_key );
        }
        else
        {
            MTERROR( mtlog, "Unknown or missing request type: " << t_request_type );
            f_return = RETURN_ERROR;
            return;
        }

        if( t_request == NULL )
        {
            MTERROR( mtlog, "Unable to create request" );
            f_return = RETURN_ERROR;
            return;
        }

        MTDEBUG( mtlog, "Sending message w/ msgop = " << t_request->get_message_op() );

        std::string t_consumer_tag;
        t_request->do_publish( f_channel, f_exchange, t_consumer_tag );

        if( ! t_consumer_tag.empty() )  // then we should wait for a reply
        {
            MTINFO( mtlog, "Waiting for a reply from the server; use ctrl-c to cancel" );

            // blocking call to wait for incoming message
            AmqpClient::Envelope::ptr_t t_envelope = f_channel->BasicConsumeMessage( t_consumer_tag );

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

    msg_request* run_client::create_run_request( const std::string& a_routing_key )
    {
        if( ! f_config.has( "file" ) )
        {
            MTERROR( mtlog, "The filename to be saved must be specified with the \"file\" option" );
            return NULL;
        }

        param_node* t_payload_node = new param_node( f_config ); // copy of f_config, which should consist of only the request arguments
        t_payload_node->add( "file", *f_config.at( "file ") ); // copy the file node
        if( f_config.has( "description" ) ) t_payload_node->add( "description", *f_config.at( "description" ) ); // (optional) copy the description node

        return msg_request::create( t_payload_node, OP_RUN, a_routing_key, message::k_json );
    }

    msg_request* run_client::create_get_request( const std::string& a_routing_key )
    {
        param_node* t_payload_node = new param_node();

        if( f_config.has( "value" ) )
        {
            param_array* t_values_array = new param_array();
            t_values_array->push_back( f_config.remove( "value" ) );
            t_payload_node->add( "values", t_values_array );
        }

        return msg_request::create( t_payload_node, OP_GET, a_routing_key, message::k_json );
    }

    msg_request* run_client::create_set_request( const std::string& a_routing_key )
    {
        if( ! f_config.has( "value" ) )
        {
            MTERROR( mtlog, "No \"value\" option given" );
            return NULL;
        }

        param_array* t_values_array = new param_array();
        t_values_array->push_back( f_config.remove( "value" ) );

        param_node* t_payload_node = new param_node();
        t_payload_node->add( "values", t_values_array );

        return msg_request::create( t_payload_node, OP_SET, a_routing_key, message::k_json );
    }

    msg_request* run_client::create_cmd_request( const std::string& a_routing_key )
    {
        param_node* t_payload_node = new param_node();

        // for the load instruction, the instruction node should be replaced by the contents of the file specified
        if( f_config.has( "load" ) )
        {
            if( ! f_config.node_at( "load" )->has( "json" ) )
            {
                MTERROR( mtlog, "Load instruction did not contain a valid file type");
                delete t_payload_node;
                return NULL;
            }

            string t_load_filename( f_config.node_at( "load" )->get_value( "json" ) );
            param_node* t_node_from_file = param_input_json::read_file( t_load_filename );
            if( t_node_from_file == NULL )
            {
                MTERROR( mtlog, "Unable to read JSON file <" << t_load_filename << ">" );
                delete t_payload_node;
                return NULL;
            }

            t_payload_node->merge( *t_node_from_file );
            f_config.erase( "load" );
        }

        // at this point, all that remains in f_config should be other options that we want to add to the payload node
        t_payload_node->merge( f_config ); // copy f_config

        return msg_request::create( t_payload_node, OP_CMD, a_routing_key, message::k_json );
    }

/*
    param_node* run_client::create_sender_info() const
    {
        param_node* t_sender_node = new param_node();
        t_sender_node->add( "commit", param_value( f_version->commit() ) );
        t_sender_node->add( "exe", param_value( f_version->exe_name() ) );
        t_sender_node->add( "version", param_value( f_version->version_str() ) );
        t_sender_node->add( "package", param_value( f_version->package() ) );
        t_sender_node->add( "hostname", param_value( f_version->hostname() ) );
        t_sender_node->add( "username", param_value( f_version->username() ) );
        return t_sender_node;
    }
*/


} /* namespace mantis */
