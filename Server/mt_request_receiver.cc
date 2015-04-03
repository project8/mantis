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
#include "mt_run_database.hh"
#include "mt_run_description.hh"
#include "mt_version.hh"

#include "M3Version.hh"

#include <cstddef>
#include <signal.h>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( const param_node& a_config, device_manager* a_dev_mgr, run_database* a_run_database, condition* a_queue_condition, const string& a_exe_name ) :
            f_master_server_config( a_config ),
            f_broker( NULL ),
            f_dev_mgr( a_dev_mgr ),
            f_run_database( a_run_database ),
            f_queue_condition( a_queue_condition ),
            f_exe_name( a_exe_name ),
            f_canceled( false )
    {
    }

    request_receiver::~request_receiver()
    {
    }

    void request_receiver::execute()
    {
        connection* t_connection = NULL;
        std::string t_consumer_tag;
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

            std::string t_exchange_name( t_broker_node->get_value( "exchange" ) );
            std::string t_queue_name( t_broker_node->get_value( "queue" ) );

            f_broker->get_connection().amqp()->DeclareExchange( t_exchange_name, AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, false, false, false );

            f_broker->get_connection().amqp()->DeclareQueue( t_queue_name, false, false, true, true );
            f_broker->get_connection().amqp()->BindQueue( t_queue_name, t_exchange_name, t_queue_name ); // queue name used for the queue and the routing key

            t_consumer_tag = f_broker->get_connection().amqp()->BasicConsume( t_queue_name, t_queue_name, true, false ); // route name used for the queue and the consumer tag; second bool is setting no_ack to false
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
            AmqpClient::Envelope::ptr_t t_envelope = f_broker->get_connection().amqp()->BasicConsumeMessage( t_consumer_tag );


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

            MTDEBUG( mtlog, "Message received:\n" << *t_msg_node );

            const param_node* t_msg_payload = t_msg_node->node_at( "payload" );
            if( t_msg_payload == NULL )
            {
                MTERROR( mtlog, "There was no payload present in the message" );
                delete t_msg_node;
                f_broker->get_connection().amqp()->BasicAck( t_envelope );
                continue;
            }

            switch( t_msg_node->get_value< unsigned >( "msgop", OP_UNKNOWN ) )
            {
                case OP_RUN:
                {
                    do_run_request( *t_msg_payload, t_envelope );
                    break;
                }
                case OP_GET:
                {
                    do_get_request( *t_msg_payload, t_envelope );
                    break;
                } // end "get" operation
                case OP_SET:
                {
                    do_set_request( *t_msg_payload, t_envelope );
                    break;
                } // end "set" operation
                default:
                    MTERROR( mtlog, "Unrecognized message operation: <" << t_msg_node->get_value< unsigned >( "msgop" ) << ">" );
                    break;
            } // end switch on message type

            // nothing should happen after the switch block except deleting objects
            delete t_msg_node;
        } // end while (true)

        MTDEBUG( mtlog, "Request receiver is done" );

        delete t_connection;

        return;
    }

    void request_receiver::apply_config( const std::string& a_config_addr, const param_value& a_value )
    {
        f_msc_mutex.lock();
        f_master_server_config.replace( a_config_addr, a_value );
        f_msc_mutex.unlock();
        return;
    }

    bool request_receiver::do_run_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope )
    {
        MTDEBUG( mtlog, "Run operation request received" );

        // required
        const param_node* t_file_node = a_msg_payload.node_at( "file" );
        if( t_file_node == NULL )
        {
            MTERROR( mtlog, "No file configuration present; aborting request" );
            f_broker->get_connection().amqp()->BasicAck( a_envelope );
            return false;
        }

        // optional
        const param_node* t_client_node = a_msg_payload.node_at( "client" );

        run_description* t_run_desc = new run_description();
        t_run_desc->set_status( run_description::created );

        t_run_desc->set_file_config( *t_file_node );

        t_run_desc->set_mantis_server_commit( TOSTRING(Mantis_GIT_COMMIT) );
        t_run_desc->set_mantis_server_exe( f_exe_name );
        t_run_desc->set_mantis_server_version( TOSTRING(Mantis_VERSION) );
        t_run_desc->set_monarch_commit( TOSTRING(Monarch_GIT_COMMIT) );
        t_run_desc->set_monarch_version( TOSTRING(Monarch_VERSION ) );

        f_msc_mutex.lock();
        t_run_desc->set_mantis_config( f_master_server_config );
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

        f_broker->get_connection().amqp()->BasicAck( a_envelope );
        t_run_desc->set_status( run_description::acknowledged );

        MTINFO( mtlog, "Queuing request" );
        f_run_database->enqueue( t_run_desc );

        // if the queue condition is waiting, release it
        if( f_queue_condition->is_waiting() == true )
        {
            //MTINFO( mtlog, "releasing queue condition" );
            f_queue_condition->release();
        }

        return true;
    }

    bool request_receiver::do_get_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope )
    {
        MTDEBUG( mtlog, "Get request received" );

        std::string t_query_type( a_msg_payload.get_value( "get", "" ) );
        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        if( ! a_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Query request has no reply-to" );
            return false;
        }

        std::string t_reply_to( a_envelope->Message()->ReplyTo() );

        param_node t_reply;
        if( t_query_type == "config" )
        {
            param_node t_payload_node;
            t_payload_node.add( "master-config", f_master_server_config );
            t_payload_node.add( "return-code", param_value( RETURN_SUCCESS ) );
            t_payload_node.add( "return-msg", param_value( "Get request succeeded" ) );
            t_reply.add( "payload", t_payload_node );
            t_reply.add( "msgtype", param_value( T_REPLY ) );
        }
        else if( t_query_type == "mantis" )
        {
            param_node t_payload_node;
            t_payload_node.add( "return-code", param_value( RETURN_ERROR ) );
            t_payload_node.add( "return-msg", param_value( "Query type <mantis> is not yet supported" ) );
            t_reply.add( "payload", t_payload_node );
            t_reply.add( "msgtype", param_value( T_REPLY ) );
        }
        else
        {
            param_node* t_msg_node = new param_node();
            t_msg_node->add( "error", param_value( "Unrecognized query type or no query type provided" ) );
            t_reply.add( "payload", t_msg_node );
            t_reply.add( "msgtype", param_value( T_REPLY ) );
        }

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

    bool request_receiver::do_set_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope )
    {
        MTDEBUG( mtlog, "Set request received" );

        param_node t_reply_node;
        t_reply_node.add( "return-code", param_value( RETURN_SUCCESS ) );
        t_reply_node.add( "return-msg", param_value( "Request succeeded" ) );

        string t_instruction( a_msg_payload.get_value( "action", "" ) );
        const param_node* t_instruction_node = a_msg_payload.node_at( t_instruction );
        if( t_instruction_node == NULL )
        {
            t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
            t_reply_node.value_at( "return-msg" )->set( "No set instruction was provided" );
            acknowledge_and_reply( t_reply_node, a_envelope );
            return false;
        }

        if( t_instruction == "set" )
        {
            // instruction contents modify an option (or options) in the master config
            f_master_server_config.merge( *t_instruction_node );
            t_reply_node.add( "master-config", f_master_server_config );
        }
        else if( t_instruction == "load" )
        {
            // instruction contents should replace the master config
            f_master_server_config = *t_instruction_node;
            t_reply_node.add( "master-config", f_master_server_config );
        }
        else if( t_instruction == "add" )
        {
            // add something to the master config
            if( t_instruction_node->has( "device" ) )
            {
                try
                {
                    const param_node* t_device_node = t_instruction_node->node_at( "device" );
                    string t_device_type = t_device_node->begin()->first;
                    string t_device_name = t_device_node->get_value( t_device_type );

                    // check if we have a device of this name
                    if( f_master_server_config.node_at( "devices" )->has( t_device_name ) )
                    {
                        t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                        t_reply_node.value_at( "return-msg" )->set( "The master config already has device <" + t_device_name + ">" );
                        acknowledge_and_reply( t_reply_node, a_envelope );
                        return false;
                    }

                    // get the config template from the device manager
                    param_node* t_device_config = f_dev_mgr->get_device_config( t_device_type );
                    if( t_device_config == NULL )
                    {
                        t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                        t_reply_node.value_at( "return-msg" )->set( "Did not find device of type <" + t_device_type + ">" );
                        acknowledge_and_reply( t_reply_node, a_envelope );
                        return false;
                    }
                    t_device_config->add( "type", param_value( t_device_type ) );
                    t_device_config->add( "enabled", param_value( 0 ) );

                    // add the configuration to the master config
                    f_master_server_config.node_at( "devices" )->add( t_device_name, t_device_config );
                }
                catch( exception& e )
                {
                    t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                    t_reply_node.value_at( "return-msg" )->set( "add/device instruction was not formatted properly" );
                    acknowledge_and_reply( t_reply_node, a_envelope );
                    return false;
                }
            }
            else
            {
                t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                t_reply_node.value_at( "return-msg" )->set( "Invalid set-add instruction" );
                acknowledge_and_reply( t_reply_node, a_envelope );
                return false;
            }

            t_reply_node.add( "master-config", f_master_server_config );
        }
        else if( t_instruction == "remove" )
        {
            // remove something from the master config
            if( t_instruction_node->has( "device" ) )
            {
                try
                 {
                     string t_device_name = t_instruction_node->get_value( "device" );

                     // check if we have a device of this name
                     if( ! f_master_server_config.node_at( "devices" )->has( t_device_name ) )
                     {
                         t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                         t_reply_node.value_at( "return-msg" )->set( "The master config does not have device <" + t_device_name + ">" );
                         acknowledge_and_reply( t_reply_node, a_envelope );
                         return false;
                     }

                     // add the configuration to the master config
                     f_master_server_config.node_at( "devices" )->remove( t_device_name );
                 }
                 catch( exception& e )
                 {
                     t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                     t_reply_node.value_at( "return-msg" )->set( "remove/device instruction was not formatted properly" );
                     acknowledge_and_reply( t_reply_node, a_envelope );
                     return false;
                 }
            }
            else
            {
                t_reply_node.value_at( "return-code" )->set( RETURN_ERROR );
                t_reply_node.value_at( "return-msg" )->set( "Invalid set-remove instruction" );
                acknowledge_and_reply( t_reply_node, a_envelope );
                return false;
            }

            t_reply_node.add( "master-config", f_master_server_config );
        }

        acknowledge_and_reply( t_reply_node, a_envelope );
        return true;
    }

    void request_receiver::acknowledge_and_reply( const param_node& a_reply_node, AmqpClient::Envelope::ptr_t a_envelope )
    {
        f_broker->get_connection().amqp()->BasicAck( a_envelope );

        if( ! a_envelope->Message()->ReplyToIsSet() )
        {
            MTWARN( mtlog, "Set request has no reply-to" );
            return;
        }

        std::string t_reply_to( a_envelope->Message()->ReplyTo() );

        param_node t_reply;
        t_reply.add( "payload", a_reply_node );
        t_reply.add( "msgtype", param_value( T_REPLY ) );
        //t_reply.add( "msgop", param_value() << OP_RUN ); // operations aren't used for replies
        //t_reply.add( "target", param_value() << t_reply_to );  // use of the target is now deprecated (3/12/15)
        t_reply.add( "timestamp", param_value( get_absolute_time_string() ) );

        std::string t_reply_str;
        if(! param_output_json::write_string( t_reply, t_reply_str, param_output_json::k_compact ) )
        {
            MTERROR( mtlog, "Could not convert reply to string" );
            return;
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
        }

        return;
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
