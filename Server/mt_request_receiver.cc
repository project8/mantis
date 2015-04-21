#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_broker.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_connection.hh"
#include "mt_constants.hh"
#include "mt_device_manager.hh"
#include "mt_logger.hh"
#include "mt_param_json.hh"
#include "mt_param_msgpack.hh"
#include "mt_parser.hh"
#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_version.hh"

#include "M3Version.hh"

#include <cstddef>
#include <signal.h>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( const param_node& a_config, device_manager* a_dev_mgr, acq_request_db* a_acq_request_db, condition* a_queue_condition, const string& a_exe_name ) :
            f_master_server_config( a_config ),
            f_broker( NULL ),
            f_queue_name(),
            f_consumer_tag(),
            f_dev_mgr( a_dev_mgr ),
            f_acq_request_db( a_acq_request_db ),
            f_queue_condition( a_queue_condition ),
            f_exe_name( a_exe_name ),
            f_canceled( false )
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
        std::string t_exchange_name;
        try
        {
            const param_node* t_broker_node = f_master_server_config.node_at( "amqp" );

            f_broker = broker::get_instance();

            if( ! f_broker->is_connected() )
            {
                MTERROR( mtlog, "Not connected to AMQP broker" );
                cancel();
                raise( SIGINT );
                return;
            }

            t_exchange_name = t_broker_node->get_value( "exchange" );
            f_queue_name = t_broker_node->get_value( "queue" );

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
            MTDEBUG( mtlog, "Waiting for incoming message" );
            AmqpClient::Envelope::ptr_t t_envelope = f_broker->get_connection().amqp()->BasicConsumeMessage( f_consumer_tag );


            if (f_canceled.load()) return;

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

            MTDEBUG( mtlog, "Message received:\n" <<
                     "Routing key: " << t_envelope->RoutingKey() <<
                     *t_msg_node );

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

            param_node t_reply_node;
            t_reply_node.add( "return-msg", new param_value( "" ) );
            t_reply_node.add( "content", new param_node() );

            switch( t_msg_node->get_value< unsigned >( "msgop", OP_UNKNOWN ) )
            {
                case OP_RUN:
                {
                    do_run_request( *t_msg_payload, t_envelope, t_reply_node );
                    break;
                }
                case OP_GET:
                {
                    do_get_request( *t_msg_payload, t_envelope, t_reply_node );
                    break;
                } // end "get" operation
                case OP_SET:
                {
                    do_set_request( *t_msg_payload, t_envelope, t_reply_node );
                    break;
                } // end "set" operation
                case OP_CMD:
                {
                    do_cmd_request( *t_msg_payload, t_envelope, t_reply_node );
                    break;
                }
                default:
                    MTERROR( mtlog, "Unrecognized message operation: <" << t_msg_node->get_value< unsigned >( "msgop" ) << ">" );
                    break;
            } // end switch on message type

            // nothing should happen after the switch block except deleting objects
            delete t_msg_node;
            delete t_msg_payload;
        } // end while (true)

        MTDEBUG( mtlog, "Request receiver is done" );

        MTDEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
        f_broker->get_connection().amqp()->BasicCancel( f_consumer_tag );
        f_consumer_tag.clear();

        MTDEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
        f_broker->get_connection().amqp()->DeleteQueue( f_queue_name, false );
        f_queue_name.clear();

        return;
    }

    void request_receiver::apply_config( const std::string& a_config_addr, const param_value& a_value )
    {
        f_msc_mutex.lock();
        f_master_server_config.replace( a_config_addr, a_value );
        f_msc_mutex.unlock();
        return;
    }

    bool request_receiver::do_run_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope, param_node& a_reply_node )
    {
        MTDEBUG( mtlog, "Run operation request received" );
        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        // required
        const param_value* t_file_node = a_msg_payload.value_at( "file" );
        if( t_file_node == NULL )
        {
            MTWARN( mtlog, "No or invalid file configuration present; aborting run request" );
            param_node t_reply_node;
            t_reply_node.add( "return-msg", param_value( "No file configuration present; aborting request" ) );
            acknowledge_and_reply( t_reply_node, R_MESSAGE_ERROR_BAD_PAYLOAD, a_envelope );
            return false;
        }

        // optional
        const param_node* t_client_node = a_msg_payload.node_at( "client" );

        acq_request* t_run_desc = new acq_request();
        t_run_desc->set_status( acq_request::created );

        t_run_desc->set_file_config( *t_file_node );
        if( a_msg_payload.has( "description" ) ) t_run_desc->set_description_config( *(a_msg_payload.value_at( "description" ) ) );

        t_run_desc->set_mantis_server_commit( TOSTRING(Mantis_GIT_COMMIT) );
        t_run_desc->set_mantis_server_exe( f_exe_name );
        t_run_desc->set_mantis_server_version( TOSTRING(Mantis_VERSION) );
        t_run_desc->set_monarch_commit( TOSTRING(Monarch_GIT_COMMIT) );
        t_run_desc->set_monarch_version( TOSTRING(Monarch_VERSION ) );

        f_msc_mutex.lock();
        t_run_desc->set_mantis_config( *f_master_server_config.node_at( "run" ) );
        f_msc_mutex.unlock();
        // remove non-enabled devices from the devices node
        param_node* t_dev_node = t_run_desc->node_at( "mantis-config" )->node_at( "devices" );
        std::vector< std::string > t_devs_to_remove;
        for( param_node::iterator t_node_it = t_dev_node->begin(); t_node_it != t_dev_node->end(); ++t_node_it )
        {
            try
            {
                if( ! t_node_it->second->as_node().get_value< bool >( "enabled", false ) )
                {
                    t_devs_to_remove.push_back( t_node_it->first );
                }
            }
            catch( exception& )
            {
                MTWARN( mtlog, "Found non-node param object in \"devices\"" );
            }
        }
        for( std::vector< std::string >::const_iterator it = t_devs_to_remove.begin(); it != t_devs_to_remove.end(); ++it )
        {
            t_dev_node->remove( *it );
        }


        if( t_client_node != NULL )
        {
            t_run_desc->set_client_commit( t_client_node->get_value( "commit", "N/A" ) );
            t_run_desc->set_client_exe( t_client_node->get_value( "exe", "N/A" ) );
            t_run_desc->set_client_version( t_client_node->get_value( "version", "N/A" ) );
        }
        else
        {
            t_run_desc->set_client_commit( "N/A" );
            t_run_desc->set_client_exe( "N/A" );
            t_run_desc->set_client_version( "N/A" );
        }

        t_run_desc->set_status( acq_request::acknowledged );

        a_reply_node.value_at( "return-msg")->set( "Run request succeeded" );
        a_reply_node.node_at( "content" )->merge( *t_run_desc );
        if( ! acknowledge_and_reply( a_reply_node, R_SUCCESS, a_envelope ) )
        {
            MTWARN( mtlog, "Failed to send reply regarding the run request" );
        }

        MTINFO( mtlog, "Queuing request" );
        f_acq_request_db->enqueue( t_run_desc );

        // if the queue condition is waiting, release it
        if( f_queue_condition->is_waiting() == true )
        {
            //MTINFO( mtlog, "releasing queue condition" );
            f_queue_condition->release();
        }

        return true;
    }

    bool request_receiver::do_get_request( const param_node& /*a_msg_payload*/, AmqpClient::Envelope::ptr_t a_envelope, param_node& a_reply_node )
    {
        MTDEBUG( mtlog, "Get request received" );

        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        if( ! a_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Query request has no reply-to" );
            return false;
        }

        std::string t_reply_to( a_envelope->Message()->ReplyTo() );

        parsable t_routing_key_node( a_envelope->RoutingKey() );
        MTDEBUG( mtlog, "Parsed routing key:\n" << t_routing_key_node );
        std::string t_query_type( t_routing_key_node.node_at( f_queue_name )->begin()->first );
        MTDEBUG( mtlog, "Query type: " << t_query_type );

        param_node t_reply;
        if( t_query_type == "run-config" )
        {
            a_reply_node.value_at( "return-msg")->set( "Get request succeeded" );
            a_reply_node.node_at( "content" )->merge( *f_master_server_config.node_at( "run" ) );
            return acknowledge_and_reply( a_reply_node, R_SUCCESS, a_envelope );
        }
        else if( t_query_type == "server-config" )
        {
            a_reply_node.value_at( "return-msg")->set( "Get request succeeded" );
            a_reply_node.node_at( "content" )->merge( f_master_server_config );
            return acknowledge_and_reply( a_reply_node, R_SUCCESS, a_envelope );
        }
        else if( t_query_type == "status" )
        {
            a_reply_node.value_at( "return-msg" )->set( "Query type <status> is not yet supported" );
            return acknowledge_and_reply( a_reply_node, R_MESSAGE_ERROR_BAD_PAYLOAD, a_envelope );
        }
        else
        {
            a_reply_node.value_at( "return-msg" )->set( "Unrecognized query type or no query type provided" );
            return acknowledge_and_reply( a_reply_node, R_MESSAGE_ERROR_BAD_PAYLOAD, a_envelope );
        }
    }

    bool request_receiver::do_set_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope, param_node& a_reply_node )
    {
        MTDEBUG( mtlog, "Set request received" );
        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        string t_routing_key = a_envelope->RoutingKey();
        if( t_routing_key.empty() )
        {
            a_reply_node.value_at( "return-msg" )->set( "No routing key was provided" );
            acknowledge_and_reply( a_reply_node, R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, a_envelope );
            return false;
        }

        MTDEBUG( mtlog, "Applying a setting" );
        // apply a configuration setting
        // the destination node should specify the configuration to set
        t_routing_key += string( "=" ) + a_msg_payload.array_at( "values" )->get_value( 0 );
        parsable t_routing_key_node_with_value( t_routing_key );
        MTDEBUG( mtlog, "Parsed routing key and added value:\n" << t_routing_key_node_with_value );
        f_master_server_config.node_at( "run" )->merge( *t_routing_key_node_with_value.node_at( f_queue_name ) );

        a_reply_node.value_at( "return-msg" )->set( "Request succeeded" );
        a_reply_node.node_at( "content" )->add( "master-config", f_master_server_config );

        return acknowledge_and_reply( a_reply_node, R_SUCCESS, a_envelope );
    }

    bool request_receiver::do_cmd_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope, param_node& a_reply_node )
    {
        MTDEBUG( mtlog, "Cmd request received" );
        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        string t_routing_key = a_envelope->RoutingKey();
        if( t_routing_key.empty() )
        {
            a_reply_node.value_at( "return-msg" )->set( "No routing key was provided" );
            acknowledge_and_reply( a_reply_node, R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, a_envelope );
            return false;
        }
        parsable t_routing_key_node( t_routing_key );
        MTDEBUG( mtlog, "Parsed routing key:\n" << t_routing_key_node );
        param_node t_dest_node( *t_routing_key_node.node_at( f_queue_name ) );

        string t_instruction( t_dest_node.begin()->first );
        if( t_instruction == "run-config" )
        {
            MTDEBUG( mtlog, "Loading a full configuration" );
            // payload contents should replace the run config
            (*f_master_server_config.node_at( "run" )) = a_msg_payload;
        }
        else if( t_instruction == "add" )
        {
            // add something to the master config
            if( t_dest_node.node_at( t_instruction )->has( "device" ) )
            {
                MTDEBUG( mtlog, "Attempting to add a device" );
                // it's expected that any values in the payload are digitizers to be added
                try
                {
                    param_node* t_devices_node = f_master_server_config.node_at( "run" )->node_at( "devices" );
                    for( param_node::const_iterator t_dev_it = a_msg_payload.begin(); t_dev_it != a_msg_payload.end(); ++t_dev_it )
                    {
                        string t_device_type = t_dev_it->first;
                        string t_device_name = t_dev_it->second->as_value().as_string();

                        // check if we have a device of this name
                        if( t_devices_node->has( t_device_name ) )
                        {
                            a_reply_node.value_at( "return-msg" )->set( "The master config already has device <" + t_device_name + ">" );
                            acknowledge_and_reply( a_reply_node, R_DEVICE_ERROR, a_envelope );
                            return false;
                        }

                        // get the config template from the device manager
                        param_node* t_device_config = f_dev_mgr->get_device_config( t_device_type );
                        if( t_device_config == NULL )
                        {
                            a_reply_node.value_at( "return-msg" )->set( "Did not find device of type <" + t_device_type + ">" );
                            acknowledge_and_reply( a_reply_node, R_DEVICE_ERROR, a_envelope );
                            return false;
                        }
                        t_device_config->add( "type", param_value( t_device_type ) );
                        t_device_config->add( "enabled", param_value( 0 ) );

                        // add the configuration to the master config
                        t_devices_node->add( t_device_name, t_device_config );
                    }
                }
                catch( exception& e )
                {
                    a_reply_node.value_at( "return-msg" )->set( "add.device instruction was not formatted properly" );
                    acknowledge_and_reply( a_reply_node, R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, a_envelope );
                    return false;
                }
            }
            else
            {
                a_reply_node.value_at( "return-msg" )->set( "Invalid add instruction" );
                acknowledge_and_reply( a_reply_node, R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, a_envelope );
                return false;
            }
        }
        else if( t_instruction == "remove" )
        {
            // remove something from the master config
            if( t_dest_node.node_at( t_instruction )->has( "device" ) )
            {
                MTDEBUG( mtlog, "Attempting to remove a device" );
                try
                 {
                     string t_device_name = t_dest_node.node_at( t_instruction )->node_at( "device" )->begin()->first;

                     // check if we have a device of this name
                     if( ! f_master_server_config.node_at( "run" )->node_at( "devices" )->has( t_device_name ) )
                     {
                         a_reply_node.value_at( "return-msg" )->set( "The master config does not have device <" + t_device_name + ">" );
                         acknowledge_and_reply( a_reply_node, R_DEVICE_ERROR, a_envelope );
                         return false;
                     }

                     // add the configuration to the master config
                     f_master_server_config.node_at( "run" )->node_at( "devices" )->erase( t_device_name );
                 }
                 catch( exception& e )
                 {
                     a_reply_node.value_at( "return-msg" )->set( "remove/device instruction was not formatted properly" );
                     acknowledge_and_reply( a_reply_node, R_MESSAGE_ERROR_INVALID_VALUE, a_envelope );
                     return false;
                 }
            }
            else
            {
                a_reply_node.value_at( "return-msg" )->set( "Invalid remove instruction" );
                acknowledge_and_reply( a_reply_node, R_AMQP_ERROR_ROUTINGKEY_NOTFOUND, a_envelope );
                return false;
            }
        }
        else
        {
            MTWARN( mtlog, "Instruction <" << t_instruction << "> not understood" );
            a_reply_node.value_at( "return-msg" )->set( "Instruction <" + t_instruction + "> not understood" );
            acknowledge_and_reply( a_reply_node, R_MESSAGE_ERROR_BAD_PAYLOAD, a_envelope );
            return false;
        }

        a_reply_node.value_at( "return-msg" )->set( "Request succeeded" );
        a_reply_node.node_at( "content" )->merge( *f_master_server_config.node_at( "run" ) );

        return acknowledge_and_reply( a_reply_node, R_SUCCESS, a_envelope );
    }

    bool request_receiver::acknowledge_and_reply( const param_node& a_reply_node, unsigned a_return_code, AmqpClient::Envelope::ptr_t a_envelope )
    {
        if( ! a_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Set request has no reply-to" );
            return false;
        }

        std::string t_reply_to( a_envelope->Message()->ReplyTo() );

        param_node t_reply;
        t_reply.add( "msgtype", param_value( T_REPLY ) );
        t_reply.add( "retcode", param_value( a_return_code ) );
        t_reply.add( "payload", a_reply_node );
        //t_reply.add( "msgop", param_value() << OP_RUN ); // operations aren't used for replies
        //t_reply.add( "target", param_value() << t_reply_to );  // use of the target is now deprecated (3/12/15)
        t_reply.add( "timestamp", param_value( get_absolute_time_string() ) );

        std::string t_reply_str;
        if(! param_output_json::write_string( t_reply, t_reply_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert reply to string" );
            return false;
        }

        AmqpClient::BasicMessage::ptr_t t_reply_msg = AmqpClient::BasicMessage::Create( t_reply_str );
        t_reply_msg->ContentEncoding( "application/json" );
        t_reply_msg->CorrelationId( a_envelope->Message()->CorrelationId() );

        try
        {
            f_broker->get_connection().amqp()->BasicPublish( "", a_envelope->Message()->ReplyTo(), t_reply_msg );
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
        if (! f_canceled.load())
        {
            f_canceled.store(true);
            return;
        }
        return;
    }

}
