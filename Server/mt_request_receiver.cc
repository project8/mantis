#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_broker.hh"
#include "mt_buffer.hh"
#include "mt_config_manager.hh"
#include "mt_connection.hh"
#include "mt_constants.hh"
#include "mt_device_manager.hh"
#include "mt_logger.hh"
#include "mt_param_json.hh"
#include "mt_param_msgpack.hh"
#include "mt_parser.hh"
#include "mt_run_server.hh"
#include "mt_version.hh"

#include "M3Version.hh"

#include <cstddef>
#include <signal.h>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    bool request_reply_package::send_reply( unsigned a_return_code, const std::string& a_return_msg )
    {
        return f_request_receiver->send_reply( a_return_code, a_return_msg, *this );
    }


    request_receiver::request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db ) :
            f_broker( NULL ),
            f_queue_name(),
            f_consumer_tag(),
            f_run_server( a_run_server ),
            f_conf_mgr( a_conf_mgr ),
            f_acq_request_db( a_acq_request_db ),
            f_canceled( false ),
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
            f_broker = broker::get_instance();

            if( ! f_broker->is_connected() )
            {
                MTERROR( mtlog, "Not connected to AMQP broker" );
                cancel();
                raise( SIGINT );
                return;
            }

            param_node* t_broker_node = f_conf_mgr->copy_master_server_config( "amqp" );

            t_exchange_name = t_broker_node->get_value( "exchange" );
            f_queue_name = t_broker_node->get_value( "queue" );

            delete t_broker_node;

            f_broker->get_connection().amqp()->DeclareExchange( t_exchange_name, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, false, false, false );

            f_broker->get_connection().amqp()->DeclareQueue( f_queue_name, false, false, true, true );
            f_broker->get_connection().amqp()->BindQueue( f_queue_name, t_exchange_name, f_queue_name + string( ".#" ) ); // all routing keys prepended with the queue name are accepted

            f_consumer_tag = f_broker->get_connection().amqp()->BasicConsume( f_queue_name, f_queue_name, true, false ); // route name used for the queue and the consumer tag; second bool is setting no_ack to false
        }
        catch( AmqpClient::AmqpException& e )
        {
            MTERROR( mtlog, "AMQP exception caught: " << e.what() );
            cancel();
            raise( SIGINT );
            return;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "Standard exception caught: " << e.what() );
            cancel();
            raise( SIGINT );
            return;
        }

        while( true )
        {
            if( f_canceled.load() ) return;

            // blocking call to wait for incoming message
            f_status.store( k_listening );
            MTINFO( mtlog, "Waiting for incoming message" );
            AmqpClient::Envelope::ptr_t t_envelope = f_broker->get_connection().amqp()->BasicConsumeMessage( f_consumer_tag );

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
                f_broker->get_connection().amqp()->BasicAck( t_envelope );
                continue;
            }

            if( t_msg_node == NULL )
            {
                MTERROR( mtlog, "Message body could not be parsed; skipping request" );
                f_broker->get_connection().amqp()->BasicAck( t_envelope );
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
                f_broker->get_connection().amqp()->BasicAck( t_envelope );
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

            f_broker->get_connection().amqp()->BasicAck( t_envelope );

            param_node t_reply_node;
            t_reply_node.add( "return-msg", new param_value( "(no message provided)" ) );
            t_reply_node.add( "content", new param_node() );

            request_reply_package t_reply_pkg( t_envelope, t_reply_node, this );

            switch( t_msg_node->get_value< unsigned >( "msgop", OP_UNKNOWN ) )
            {
                case OP_RUN:
                {
                    do_run_request( *t_msg_payload, t_routing_key, t_reply_pkg );
                    break;
                }
                case OP_GET:
                {
                    do_get_request( *t_msg_payload, t_routing_key, t_reply_pkg );
                    break;
                } // end "get" operation
                case OP_SET:
                {
                    do_set_request( *t_msg_payload, t_routing_key, t_reply_pkg );
                    break;
                } // end "set" operation
                case OP_CMD:
                {
                    do_cmd_request( *t_msg_payload, t_routing_key, t_reply_pkg );
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
        f_broker->get_connection().amqp()->BasicCancel( f_consumer_tag );
        f_consumer_tag.clear();

        MTDEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
        f_broker->get_connection().amqp()->DeleteQueue( f_queue_name, false );
        f_queue_name.clear();

        return;
    }

    bool request_receiver::do_run_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Run operation request received" );

        return f_acq_request_db->handle_new_acq_request( a_msg_payload, a_mantis_routing_key, a_pkg );
    }

    bool request_receiver::do_get_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
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
            return f_conf_mgr->handle_get_acq_config_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "server-config" )
        {
            return f_conf_mgr->handle_get_server_config_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "acq-status" )
        {
            return f_acq_request_db->handle_get_acq_status_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "queue" )
        {
            return f_acq_request_db->handle_queue_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "queue-size" )
        {
            return f_acq_request_db->handle_queue_size_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_query_type == "server-status" )
        {
            return f_run_server->handle_get_server_status_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Unrecognized query type or no query type provided", a_pkg );
            return false;
        }
    }

    bool request_receiver::do_set_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Set request received" );

        return f_conf_mgr->handle_set_request( a_msg_payload, a_mantis_routing_key, a_pkg );
    }

    bool request_receiver::do_cmd_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
    {
        MTDEBUG( mtlog, "Cmd request received" );

        std::string t_instruction;
        try
        {
            parsable t_routing_key_node( a_mantis_routing_key );
            t_instruction = t_routing_key_node.begin()->first;
            MTDEBUG( mtlog, "I type: " << t_instruction );
        }
        catch( exception& e )
        {
            send_reply( R_DEVICE_ERROR, string( "Routing key was not formatted correctly: " ) + e.what(), a_pkg );
            return false;
        }

        if( t_instruction == "replace-config" )
        {
            return f_conf_mgr->handle_replace_acq_config( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "add" )
        {
            return f_conf_mgr->handle_add_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "remove" )
        {
            return f_conf_mgr->handle_remove_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "cancel-acq" )
        {
            return f_acq_request_db->handle_cancel_acq_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "clear-queue" )
        {
            return f_acq_request_db->handle_clear_queue_request( a_msg_payload, a_mantis_routing_key, a_pkg );
        }
        else if( t_instruction == "start-queue" )
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Command type <start> is not yet supported", a_pkg );
            return false;
        }
        else if( t_instruction == "stop-queue" )
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Command type <stop> is not yet supported", a_pkg );
            return false;
        }
        else if( t_instruction == "stop-acq" )
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Command type <stop> is not yet supported", a_pkg );
            return false;
        }
        else if( t_instruction == "stop-all" )
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Command type <stop> is not yet supported", a_pkg );
            return false;
        }
        else if( t_instruction == "quit-mantis" )
        {
            send_reply( R_MESSAGE_ERROR_BAD_PAYLOAD, "Command type <server-status> is not yet supported", a_pkg );
            return false;
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

        std::string t_reply_to( a_pkg.f_envelope->Message()->ReplyTo() );

        a_pkg.f_reply_node.replace( "return-msg", param_value( a_return_msg ) );

        param_node t_reply;
        t_reply.add( "msgtype", new param_value( T_REPLY ) );
        t_reply.add( "retcode", new param_value( a_return_code ) );
        t_reply.add( "payload", a_pkg.f_reply_node );
        t_reply.add( "timestamp", param_value( get_absolute_time_string() ) );

        MTDEBUG( mtlog, "Sending reply message:\n" << t_reply );

        std::string t_reply_str;
        if(! param_output_json::write_string( t_reply, t_reply_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert reply to string" );
            return false;
        }

        AmqpClient::BasicMessage::ptr_t t_reply_msg = AmqpClient::BasicMessage::Create( t_reply_str );
        t_reply_msg->ContentEncoding( "application/json" );
        t_reply_msg->CorrelationId( a_pkg.f_envelope->Message()->CorrelationId() );

        try
        {
            f_broker->get_connection().amqp()->BasicPublish( "", a_pkg.f_envelope->Message()->ReplyTo(), t_reply_msg );
        }
        catch( AmqpClient::MessageReturnedException& e )
        {
            MTERROR( mtlog, "Reply message could not be sent: " << e.what() );
            return false;
        }

        return true;
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
