#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_buffer.hh"
#include "mt_config_manager.hh"
#include "mt_constants.hh"
#include "mt_device_manager.hh"
#include "mt_logger.hh"
#include "mt_message.hh"
#include "mt_param_json.hh"
#include "mt_param_msgpack.hh"
#include "mt_parser.hh"
#include "mt_run_server.hh"
#include "mt_server_worker.hh"

#include <cstddef>


using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    bool request_reply_package::send_reply( unsigned a_return_code, const std::string& a_return_msg )
    {
        return f_request_receiver->send_reply( a_return_code, a_return_msg, *this );
    }


    request_receiver::request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db, server_worker* a_server_worker, amqp_channel_ptr a_channel ) :
            f_channel( a_channel ),
            f_queue_name(),
            f_consumer_tag(),
            f_run_server( a_run_server ),
            f_conf_mgr( a_conf_mgr ),
            f_acq_request_db( a_acq_request_db ),
            f_server_worker( a_server_worker ),
            f_canceled( false ),
            f_lockout_tag(),
            f_lockout_key( generate_nil_uuid() ),
            f_status( k_initialized )
    {
    }

    request_receiver::~request_receiver()
    {
        /* can't do this, because the amqp connection may already be broken, in which case this results in a segmentation fault
        if( f_broker != NULL )
        {
            if( ! f_consumer_tag.empty() )
            {
                MTDEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
                f_broker->get_connection().amqp()->BasicCancel( f_consumer_tag );
                f_consumer_tag.clear();
            }
            if( ! f_queue_name.empty() )
            {
                MTDEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
                f_broker->get_connection().amqp()->DeleteQueue( f_queue_name, false );
                f_queue_name.clear();
            }
        }
        */
    }

    void request_receiver::execute()
    {
        f_status.store( k_starting );

        std::string t_exchange_name;
        try
        {
            param_node* t_broker_node = f_conf_mgr->copy_master_server_config( "amqp" );

            t_exchange_name = t_broker_node->get_value( "exchange" );
            f_queue_name = t_broker_node->get_value( "queue" );

            delete t_broker_node;

            f_channel->DeclareExchange( t_exchange_name, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, false, false, false );

            f_channel->DeclareQueue( f_queue_name, false, false, true, true );
            f_channel->BindQueue( f_queue_name, t_exchange_name, f_queue_name + string( ".#" ) ); // all routing keys prepended with the queue name are accepted

            f_consumer_tag = f_channel->BasicConsume( f_queue_name, f_queue_name, true, false ); // route name used for the queue and the consumer tag; second bool is setting no_ack to false
        }
        catch( AmqpClient::AmqpException& e )
        {
            MTERROR( mtlog, "AMQP exception caught: " << e.what() );
            //cancel();
            f_run_server->quit_server();
            return;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Standard exception caught: " << e.what() );
            //cancel();
            f_run_server->quit_server();
            return;
        }

        while( true )
        {
            if( f_canceled.load() ) return;

            // blocking call to wait for incoming message
            f_status.store( k_listening );
            MTINFO( mtlog, "Waiting for incoming message" );
            AmqpClient::Envelope::ptr_t t_envelope = f_channel->BasicConsumeMessage( f_consumer_tag );

            if (f_canceled.load()) return;

            f_status.store( k_processing );

            param_node* t_msg_node = NULL;
            if( t_envelope->Message()->ContentEncoding() == "application/json" )
            {
                t_msg_node = param_input_json::read_string( t_envelope->Message()->Body() );
            }
            else if( t_envelope->Message()->ContentEncoding() == "application/msgpack" )
            {
                t_msg_node = param_input_msgpack::read_string( t_envelope->Message()->Body() );
            }
            else
            {
                MTERROR( mtlog, "Unable to parse message with content type <" << t_envelope->Message()->ContentEncoding() << ">" );
                f_channel->BasicAck( t_envelope );
                continue;
            }

            if( t_msg_node == NULL )
            {
                MTERROR( mtlog, "Message body could not be parsed; skipping request" );
                f_channel->BasicAck( t_envelope );
                continue;
            }

            string t_routing_key = t_envelope->RoutingKey();

            MTINFO( mtlog, "Message received" );
            MTDEBUG( mtlog, "Message received:\n" <<
                     "Routing key: " << t_routing_key <<
                     *t_msg_node );

            if( t_routing_key.find( f_queue_name ) != 0 )
            {
                MTWARN( mtlog, "Routing key does not start with the queue name");
                f_channel->BasicAck( t_envelope );
                continue;
            }
            t_routing_key.erase( 0, f_queue_name.size() + 1 ); // 1 added to remove the '.' that separates nodes
            MTDEBUG( mtlog, "Mantis routing key: " << t_routing_key );


            param_node* t_msg_payload = NULL;
            param* t_payload_param = t_msg_node->remove( "payload" );
            if( t_payload_param == NULL ) t_msg_payload = new param_node();
            else
            {
                if( t_payload_param->is_node() ) t_msg_payload = static_cast< param_node* >( t_payload_param );
                else
                {
                    MTWARN( mtlog, "Non-node payload is present; it will be ignored" );
                    t_msg_payload = new param_node();
                    delete t_payload_param;
                }
            }

            const param_node* t_sender_node = t_msg_node->node_at( "sender_info" );

            f_channel->BasicAck( t_envelope );

            param_node t_reply_node;

            request_reply_package t_reply_pkg( t_envelope, t_reply_node, this );

            switch( t_msg_node->get_value< unsigned >( "msgop", OP_UNKNOWN ) )
            {
                case OP_RUN:
                {
                    do_run_request( *t_msg_payload, *t_sender_node, t_routing_key, t_reply_pkg );
                    break;
                }
                case OP_GET:
                {
                    do_get_request( *t_msg_payload, *t_sender_node, t_routing_key, t_reply_pkg );
                    break;
                } // end "get" operation
                case OP_SET:
                {
                    do_set_request( *t_msg_payload, *t_sender_node, t_routing_key, t_reply_pkg );
                    break;
                } // end "set" operation
                case OP_CMD:
                {
                    do_cmd_request( *t_msg_payload, *t_sender_node, t_routing_key, t_reply_pkg );
                    break;
                }
                default:
                    MTERROR( mtlog, "Unrecognized message operation: <" << t_msg_node->get_value< unsigned >( "msgop" ) << ">" );
                    break;
            } // end switch on message type

            // nothing should happen after the switch block except deleting objects
            delete t_msg_node;
            delete t_msg_payload;

            MTINFO( mtlog, "Message handled" );
        } // end while (true)

        f_status.store( k_done );
        MTDEBUG( mtlog, "Request receiver is done" );

        MTDEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
        f_channel->BasicCancel( f_consumer_tag );
        f_consumer_tag.clear();

        MTDEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
        f_channel->DeleteQueue( f_queue_name, false );
        f_queue_name.clear();

        return;
    }

    bool request_receiver::do_run_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Run operation request received" );

        if( ! authenticate( uuid_from_string( a_msg_payload.get_value( "key", "") ) ) )
        {
            string t_key_used( a_msg_payload.get_value( "key", "" ) );
            MTINFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_DEVICE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        return f_acq_request_db->handle_new_acq_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
    }

    bool request_receiver::do_get_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Get request received" );

        // require that there is a reply-to
        if( ! a_pkg.f_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Query request has no reply-to" );
            return false;
        }

        std::string t_query_type;
        try
        {
            parsable t_routing_key_node( a_mantis_routing_key );
            t_query_type = t_routing_key_node.begin()->first;
            MTDEBUG( mtlog, "Query type: " << t_query_type );
        }
        catch( exception& e )
        {
            send_reply( R_DEVICE_ERROR, string( "Routing key was not formatted correctly: " ) + e.what(), a_pkg );
            return false;
        }

        param_node t_reply;
        if( t_query_type == "acq-config" )
        {
            return f_conf_mgr->handle_get_acq_config_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "server-config" )
        {
            return f_conf_mgr->handle_get_server_config_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "is-locked" )
        {
            return handle_is_locked_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "acq-status" )
        {
            return f_acq_request_db->handle_get_acq_status_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "queue" )
        {
            return f_acq_request_db->handle_queue_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "queue-size" )
        {
            return f_acq_request_db->handle_queue_size_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "server-status" )
        {
            return f_run_server->handle_get_server_status_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Unrecognized query type or no query type provided", a_pkg );
            return false;
        }
    }

    bool request_receiver::do_set_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Set request received" );

        if( ! authenticate( uuid_from_string( a_msg_payload.get_value( "key", "") ) ) )
        {
            string t_key_used( a_msg_payload.get_value( "key", "" ) );
            MTINFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_DEVICE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        return f_conf_mgr->handle_set_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
    }

    bool request_receiver::do_cmd_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Cmd request received" );

        //MTWARN( mtlog, "uuid string: " << a_msg_payload.get_value( "key", "") << ", uuid: " << uuid_from_string( a_msg_payload.get_value( "key", "") ) );
        if( ! authenticate( uuid_from_string( a_msg_payload.get_value( "key", "") ) ) )
        {
            string t_key_used( a_msg_payload.get_value( "key", "" ) );
            MTINFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_DEVICE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        std::string t_instruction;
        if( ! a_mantis_routing_key.empty() )
        {
            try
            {
                parsable t_routing_key_node( a_mantis_routing_key );
                t_instruction = t_routing_key_node.begin()->first;
                MTDEBUG( mtlog, "Instruction: " << t_instruction );
            }
            catch( exception& e )
            {
                send_reply( R_DEVICE_ERROR, string( "Routing key was not formatted correctly: " ) + e.what(), a_pkg );
                return false;
            }
        }
        else
        {
            if( ! a_msg_payload.has( "values" ) || ! a_msg_payload.at( "values" )->is_array() )
            {
                send_reply( R_DEVICE_ERROR, string( "No values array present (required for cmd instruction)" ), a_pkg );
                return false;
            }
            t_instruction = a_msg_payload.array_at( "values" )->get_value( 0 );
        }

        if( t_instruction == "replace-config" )
        {
            return f_conf_mgr->handle_replace_acq_config( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "add" )
        {
            return f_conf_mgr->handle_add_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "remove" )
        {
            return f_conf_mgr->handle_remove_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "lock" )
        {
            return handle_lock_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "unlock" )
        {
            return handle_unlock_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "cancel-acq" )
        {
            return f_acq_request_db->handle_cancel_acq_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "clear-queue" )
        {
            return f_acq_request_db->handle_clear_queue_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "start-queue" )
        {
            return f_acq_request_db->handle_start_queue_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "stop-queue" )
        {
            return f_acq_request_db->handle_stop_queue_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "stop-acq" )
        {
            return f_server_worker->handle_stop_acq_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "stop-all" )
        {
            return f_run_server->handle_stop_all_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "quit-mantis" )
        {
            return f_run_server->handle_quit_server_request( a_msg_payload, a_sender_node, a_mantis_routing_key, a_pkg );
        }
        else
        {
            MTWARN( mtlog, "Instruction <" << t_instruction << "> not understood" );
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Instruction <" + t_instruction + "> not understood", a_pkg );
            return false;
        }
    }

    bool request_receiver::send_reply( unsigned a_return_code, const std::string& a_return_msg, request_reply_package& a_pkg ) const
    {
        if( ! a_pkg.f_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Set request has no reply-to" );
            return false;
        }

        msg_reply* t_reply = msg_reply::create( a_return_code, a_return_msg, new param_node( a_pkg.f_reply_node ), a_pkg.f_envelope->Message()->ReplyTo(), message::k_json );
        t_reply->set_correlation_id( a_pkg.f_envelope->Message()->CorrelationId() );

        MTDEBUG( mtlog, "Sending reply message:\n" <<
                 "Return code: " << t_reply->get_return_code() << '\n' <<
                 "Return message: " << t_reply->get_return_message() <<
                 a_pkg.f_reply_node );

        string t_consumer_tag;
        if( ! t_reply->do_publish( f_channel, "", t_consumer_tag ) )
        {
            MTWARN( mtlog, "Unable to send reply" );
            return false;
        }

        return true;
    }
/*
    param_node* request_receiver::create_sender_info() const
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

    bool request_receiver::handle_lock_request( const param_node& /*a_msg_payload*/, const param_node& a_sender_node, const std::string& /*a_mantis_routing_key*/, request_reply_package& a_pkg )
    {
        key_t t_new_key = enable_lockout( a_sender_node );
        if( t_new_key.is_nil() )
        {
            a_pkg.send_reply( R_DEVICE_ERROR, "Unable to lock server" );
            return false;
        }

        a_pkg.f_reply_node.add( "key", new param_value( string_from_uuid( t_new_key ) ) );
        return a_pkg.send_reply( R_SUCCESS, "Server is now locked" );
    }

    bool request_receiver::handle_unlock_request( const param_node& a_msg_payload, const param_node& /*a_sender_node*/, const std::string& /*a_mantis_routing_key*/, request_reply_package& a_pkg )
    {
        if( ! is_locked() )
        {
            return a_pkg.send_reply( R_WARNING_NO_ACTION_TAKEN, "Already unlocked" );
        }

        string t_key_string = a_msg_payload.get_value( "key", "" );
        if( t_key_string.empty() )
        {
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "No key provided" );
            return false;
        }
        bool t_force = a_msg_payload.get_value( "force", false );

        if( disable_lockout( uuid_from_string( t_key_string ), t_force ) )
        {
            return a_pkg.send_reply( R_SUCCESS, "Server unlocked" );
        }
        a_pkg.send_reply( R_DEVICE_ERROR, "Failed to unlock server" );
        return false;
    }

    bool request_receiver::handle_is_locked_request( const param_node& /*a_msg_payload*/, const param_node& /*a_sender_node*/, const std::string& /*a_mantis_routing_key*/, request_reply_package& a_pkg )
    {
        bool t_is_locked = is_locked();
        a_pkg.f_reply_node.add( "is_locked", param_value( t_is_locked ) );
        if( t_is_locked ) a_pkg.f_reply_node.add( "tag", f_lockout_tag );
        return a_pkg.send_reply( R_SUCCESS, "Checked lock status" );
    }


    void request_receiver::cancel()
    {
        MTDEBUG( mtlog, "Canceling request receiver" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_status.store( k_canceled );
            return;
        }
        return;
    }

    request_receiver::key_t request_receiver::enable_lockout( const param_node& a_tag )
    {
        if( is_locked() ) return generate_nil_uuid();
        f_lockout_key = generate_random_uuid();
        f_lockout_tag = a_tag;
        return f_lockout_key;
    }

    bool request_receiver::disable_lockout( const key_t& a_key, bool a_force )
    {
        if( ! is_locked() ) return true;
        if( ! a_force && a_key != f_lockout_key ) return false;
        f_lockout_key = generate_nil_uuid();
        f_lockout_tag.clear();
        return true;
    }

    bool request_receiver::is_locked() const
    {
        return ! f_lockout_key.is_nil();
    }

    const param_node& request_receiver::get_lockout_tag() const
    {
        return f_lockout_tag;
    }

    bool request_receiver::check_key( const key_t& a_key ) const
    {
        return f_lockout_key == a_key;
    }

    bool request_receiver::authenticate( const key_t& a_key ) const
    {
        MTDEBUG( mtlog, "Authenticating with key <" << a_key << ">" );
        if( is_locked() ) return check_key( a_key );
        return true;
    }

    std::string request_receiver::interpret_status( status a_status )
    {
        switch( a_status )
        {
            case k_initialized:
                return std::string( "Initialized" );
                break;
            case k_starting:
                return std::string( "Starting" );
                break;
            case k_processing:
                return std::string( "Processing" );
                break;
            case k_canceled:
                return std::string( "Canceled" );
                break;
            case k_done:
                return std::string( "Done" );
                break;
            case k_error:
                return std::string( "Error" );
                break;
            default:
                return std::string( "Unknown" );
        }
    }


}
