#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_buffer.hh"
#include "mt_config_manager.hh"
#include "mt_constants.hh"
#include "mt_exception.hh"
#include "mt_message.hh"
#include "mt_parser.hh"
#include "mt_run_server.hh"
#include "mt_server_worker.hh"

#include "logger.hh"
#include "parsable.hh"

#include <cstddef>
#include <sstream>




namespace mantis
{
    using std::string;

    using scarab::parsable;

    LOGGER( mtlog, "request_receiver" );

    bool request_reply_package::send_reply( unsigned a_return_code, const std::string& a_return_msg )
    {
        msg_reply* t_reply = msg_reply::create( a_return_code, a_return_msg, new param_node( f_payload ), f_request->get_reply_to(), "", message::k_json );
        t_reply->set_correlation_id( f_request->get_correlation_id() );

        DEBUG( mtlog, "Sending reply message:\n" <<
                 "Return code: " << t_reply->get_return_code() << '\n' <<
                 "Return message: " << t_reply->get_return_message() <<
                 f_payload );

        string t_consumer_tag;
        if( ! t_reply->do_publish( f_channel, f_exchange, t_consumer_tag ) )
        {
            WARN( mtlog, "Unable to send reply" );
            return false;
        }

        return true;
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
                DEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
                f_broker->get_connection().amqp()->BasicCancel( f_consumer_tag );
                f_consumer_tag.clear();
            }
            if( ! f_queue_name.empty() )
            {
                DEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
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
            ERROR( mtlog, "AMQP exception caught: " << e.what() );
            //cancel();
            f_run_server->quit_server();
            return;
        }
        catch( std::exception& e )
        {
            ERROR( mtlog, "Standard exception caught: " << e.what() );
            //cancel();
            f_run_server->quit_server();
            return;
        }

        while( true )
        {
            if( f_canceled.load() ) return;

            // blocking call to wait for incoming message
            f_status.store( k_listening );
            INFO( mtlog, "Waiting for incoming message" );
            AmqpClient::Envelope::ptr_t t_envelope = f_channel->BasicConsumeMessage( f_consumer_tag );

            f_channel->BasicAck( t_envelope );
            INFO( mtlog, "Message received" );

            if (f_canceled.load()) return;

            f_status.store( k_processing );

            message* t_message = message::process_envelope( t_envelope, f_queue_name );

            if( t_message->is_request() )
            {
                msg_request* t_request = static_cast< msg_request* >( t_message );
                request_reply_package t_reply_pkg( t_request, f_channel, t_exchange_name );

                // the lockout key must be valid
                if( ! t_request->get_lockout_key_valid() )
                {
                    t_reply_pkg.send_reply( R_MESSAGE_ERROR_INVALID_KEY, "Lockout key could not be parsed" );
                    WARN( mtlog, "Message had an invalid lockout key" );
                }
                else
                {
                    switch( t_request->get_message_op() )
                    {
                        case OP_RUN:
                        {
                            do_run_request( t_request, t_reply_pkg );
                            break;
                        }
                        case OP_GET:
                        {
                            do_get_request( t_request, t_reply_pkg );
                            break;
                        } // end "get" operation
                        case OP_SET:
                        {
                            do_set_request( t_request, t_reply_pkg );
                            break;
                        } // end "set" operation
                        case OP_CMD:
                        {
                            do_cmd_request( t_request, t_reply_pkg );
                            break;
                        }
                        default:
                            std::stringstream t_error_stream;
                            t_error_stream << "Unrecognized message operation: <" << t_request->get_message_type() << ">";
                            string t_error_msg( t_error_stream.str() );
                            ERROR( mtlog, t_error_msg );
                            t_reply_pkg.send_reply( R_MESSAGE_ERROR_INVALID_METHOD, t_error_msg );
                            break;
                    } // end switch on message type
                }
            }
            else
            {
                WARN( mtlog, "Non-request message received by the request_receiver, and will be ignored" );
            }

            delete t_message;

            INFO( mtlog, "Message handled" );
        } // end while (true)

        f_status.store( k_done );
        DEBUG( mtlog, "Request receiver is done" );

        DEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
        f_channel->BasicCancel( f_consumer_tag );
        f_consumer_tag.clear();

        DEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
        f_channel->DeleteQueue( f_queue_name, false );
        f_queue_name.clear();

        return;
    }

    bool request_receiver::do_run_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        DEBUG( mtlog, "Run operation request received" );

        if( ! authenticate( a_request->get_lockout_key() ) )
        {
            string t_key_used( a_request->get_payload().get_value( "key", "" ) );
            INFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_MESSAGE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        return f_acq_request_db->handle_new_acq_request( a_request, a_pkg );
    }

    bool request_receiver::do_get_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        DEBUG( mtlog, "Get request received" );

        // require that there is a reply-to
        if( a_request->get_reply_to().empty() )
        {
            WARN( mtlog, "Get request has no reply-to" );
            return false;
        }

        std::string t_query_type;
        try
        {
            parsable t_routing_key_node( a_request->get_mantis_routing_key() );
            t_query_type = t_routing_key_node.begin()->first;
            DEBUG( mtlog, "Query type: " << t_query_type );
        }
        catch( exception& e )
        {
            a_pkg.send_reply( R_DEVICE_ERROR, string( "Routing key was not formatted correctly: " ) + e.what() );
            return false;
        }

        param_node t_reply;
        if( t_query_type == "acq-config" )
        {
            return f_conf_mgr->handle_get_acq_config_request( a_request, a_pkg );
        }
        else if( t_query_type == "server-config" )
        {
            return f_conf_mgr->handle_get_server_config_request( a_request, a_pkg );
        }
        else if( t_query_type == "is-locked" )
        {
            return handle_is_locked_request( a_request, a_pkg );
        }
        else if( t_query_type == "acq-status" )
        {
            return f_acq_request_db->handle_get_acq_status_request( a_request, a_pkg );
        }
        else if( t_query_type == "queue" )
        {
            return f_acq_request_db->handle_queue_request( a_request, a_pkg );
        }
        else if( t_query_type == "queue-size" )
        {
            return f_acq_request_db->handle_queue_size_request( a_request, a_pkg );
        }
        else if( t_query_type == "server-status" )
        {
            return f_run_server->handle_get_server_status_request( a_request, a_pkg );
        }
        else
        {
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Unrecognized query type or no query type provided" );
            return false;
        }
    }

    bool request_receiver::do_set_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        DEBUG( mtlog, "Set request received" );

        if( ! authenticate( a_request->get_lockout_key() ) )
        {
            string t_key_used( a_request->get_payload().get_value( "key", "" ) );
            INFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_MESSAGE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        return f_conf_mgr->handle_set_request( a_request, a_pkg );
    }

    bool request_receiver::do_cmd_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        DEBUG( mtlog, "Cmd request received" );

        // get the instruction before checking the lockout key authentication because we need to have the exception for
        // the unlock instruction that allows us to force the unlock.
        std::string t_instruction;
        if( ! a_request->get_mantis_routing_key().empty() )
        {
            try
            {
                parsable t_routing_key_node( a_request->get_mantis_routing_key() );
                t_instruction = t_routing_key_node.begin()->first;
                DEBUG( mtlog, "Instruction: " << t_instruction );
            }
            catch( exception& e )
            {
                a_pkg.send_reply( R_DEVICE_ERROR, string( "Routing key was not formatted correctly: " ) + e.what() );
                return false;
            }
        }
        else
        {
            if( ! a_request->get_payload().has( "values" ) || ! a_request->get_payload().at( "values" )->is_array() )
            {
                a_pkg.send_reply( R_DEVICE_ERROR, string( "No values array present (required for cmd instruction)" ) );
                return false;
            }
            t_instruction = a_request->get_payload().array_at( "values" )->get_value( 0 );
        }

        //WARN( mtlog, "uuid string: " << a_request->get_payload().get_value( "key", "") << ", uuid: " << uuid_from_string( a_request->get_payload().get_value( "key", "") ) );
        // this condition includes the exception for the unlock instruction that allows us to force the unlock regardless of the key.
        // disable_key() checks the lockout key if it's not forced, so it's okay that we bypass this call to authenticate() for the unlock instruction.
        if( ! authenticate( a_request->get_lockout_key() ) && t_instruction != "unlock" )
        {
            string t_key_used( a_request->get_payload().get_value( "key", "" ) );
            INFO( mtlog, "Request denied due to lockout (key used: " << t_key_used << ")" );
            a_pkg.send_reply( R_MESSAGE_ERROR_ACCESS_DENIED, "Request denied due to lockout (key used: " + t_key_used + ")" );
            return false;
        }

        if( t_instruction == "replace-config" )
        {
            return f_conf_mgr->handle_replace_acq_config( a_request, a_pkg );
        }
        else if( t_instruction == "add" )
        {
            return f_conf_mgr->handle_add_request( a_request, a_pkg );
        }
        else if( t_instruction == "remove" )
        {
            return f_conf_mgr->handle_remove_request( a_request, a_pkg );
        }
        else if( t_instruction == "lock" )
        {
            return handle_lock_request( a_request, a_pkg );
        }
        else if( t_instruction == "unlock" )
        {
            return handle_unlock_request( a_request, a_pkg );
        }
        else if( t_instruction == "ping" )
        {
            return handle_ping_request( a_request, a_pkg );
        }
        else if( t_instruction == "cancel-acq" )
        {
            return f_acq_request_db->handle_cancel_acq_request( a_request, a_pkg );
        }
        else if( t_instruction == "clear-queue" )
        {
            return f_acq_request_db->handle_clear_queue_request( a_request, a_pkg );
        }
        else if( t_instruction == "start-queue" )
        {
            return f_acq_request_db->handle_start_queue_request( a_request, a_pkg );
        }
        else if( t_instruction == "stop-queue" )
        {
            return f_acq_request_db->handle_stop_queue_request( a_request, a_pkg );
        }
        else if( t_instruction == "stop-acq" )
        {
            return f_server_worker->handle_stop_acq_request( a_request, a_pkg );
        }
        else if( t_instruction == "stop-all" )
        {
            return f_run_server->handle_stop_all_request( a_request, a_pkg );
        }
        else if( t_instruction == "quit-mantis" )
        {
            return f_run_server->handle_quit_server_request( a_request, a_pkg );
        }
        else
        {
            WARN( mtlog, "Instruction <" << t_instruction << "> not understood" );
            a_pkg.send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Instruction <" + t_instruction + "> not understood" );
            return false;
        }
    }


    bool request_receiver::handle_lock_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        uuid_t t_new_key = enable_lockout( a_request->get_sender_info(), a_request->get_lockout_key() );
        if( t_new_key.is_nil() )
        {
            a_pkg.send_reply( R_DEVICE_ERROR, "Unable to lock server" );
            return false;
        }

        a_pkg.f_payload.add( "key", new param_value( string_from_uuid( t_new_key ) ) );
        return a_pkg.send_reply( R_SUCCESS, "Server is now locked" );
    }

    bool request_receiver::handle_unlock_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        if( ! is_locked() )
        {
            return a_pkg.send_reply( R_WARNING_NO_ACTION_TAKEN, "Already unlocked" );
        }

        bool t_force = a_request->get_payload().get_value( "force", false );

        if( disable_lockout( a_request->get_lockout_key(), t_force ) )
        {
            return a_pkg.send_reply( R_SUCCESS, "Server unlocked" );
        }
        a_pkg.send_reply( R_DEVICE_ERROR, "Failed to unlock server" );
        return false;
    }

    bool request_receiver::handle_is_locked_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        bool t_is_locked = is_locked();
        a_pkg.f_payload.add( "is_locked", param_value( t_is_locked ) );
        if( t_is_locked ) a_pkg.f_payload.add( "tag", f_lockout_tag );
        return a_pkg.send_reply( R_SUCCESS, "Checked lock status" );
    }

    bool request_receiver::handle_ping_request( const msg_request* a_request, request_reply_package& a_pkg )
    {
        string t_sender = a_request->get_sender_package();
        return a_pkg.send_reply( R_SUCCESS, "Hello, " + t_sender );
    }


    void request_receiver::cancel()
    {
        DEBUG( mtlog, "Canceling request receiver" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_status.store( k_canceled );
            return;
        }
        return;
    }

    uuid_t request_receiver::enable_lockout( const param_node& a_tag )
    {
        return enable_lockout( a_tag, generate_random_uuid() );
    }

    uuid_t request_receiver::enable_lockout( const param_node& a_tag, uuid_t a_key )
    {
        if( is_locked() ) return generate_nil_uuid();
        if( a_key.is_nil() ) f_lockout_key = generate_random_uuid();
        else f_lockout_key = a_key;
        f_lockout_tag = a_tag;
        return f_lockout_key;
    }

    bool request_receiver::disable_lockout( const uuid_t& a_key, bool a_force )
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

    bool request_receiver::check_key( const uuid_t& a_key ) const
    {
        return f_lockout_key == a_key;
    }

    bool request_receiver::authenticate( const uuid_t& a_key ) const
    {
        DEBUG( mtlog, "Authenticating with key <" << a_key << ">" );
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
