#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_broker.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_connection.hh"
#include "mt_constants.hh"
#include "mt_logger.hh"
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

    request_receiver::request_receiver( const param_node& a_config, broker* a_broker, run_database* a_run_database, condition* a_queue_condition, const string& a_exe_name ) :
            f_master_server_config( a_config ),
            f_broker( a_broker ),
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
            t_connection = f_broker->create_connection();
            if( t_connection == NULL )
            {
                MTERROR( mtlog, "Cannot create connection to AMQP broker" );
                cancel();
                raise( SIGINT );
                return;
            }

            t_connection->amqp()->DeclareExchange( "requests", AmqpClient::Channel::EXCHANGE_TYPE_TOPIC, false, false, false );

            t_connection->amqp()->DeclareQueue( "mantis", false, false, true, true );
            t_connection->amqp()->BindQueue( "mantis", "requests", "mantis" );

            t_consumer_tag = t_connection->amqp()->BasicConsume( "mantis", "mantis", true, false ); // second bool is setting no_ack to false
        }
        catch( AmqpClient::AmqpException& e )
        {
            MTERROR( mtlog, "AMQP exception caught: " << e.what() );
            cancel();
            raise( SIGINT );
            return;
        }

        while( true )
        {
            if( f_canceled.load() ) return;

            // blocking call to wait for incoming message
            MTDEBUG( mtlog, "Waiting for incoming message" );
            AmqpClient::Envelope::ptr_t t_envelope = t_connection->amqp()->BasicConsumeMessage( t_consumer_tag );


            if (f_canceled.load()) return;

            param_node* t_msg_node = NULL;
            if( t_envelope->Message()->ContentEncoding() == "application/json" )
            {
                t_msg_node = param_input_json::read_string( t_envelope->Message()->Body() );
            }
            else
            {
                MTERROR( mtlog, "Unable to parse message with content type <" << t_envelope->Message()->ContentEncoding() << ">" );
                t_connection->amqp()->BasicAck( t_envelope );
                continue;
            }

            if( t_msg_node == NULL )
            {
                MTERROR( mtlog, "Message body could not be parsed; skipping request" );
                t_connection->amqp()->BasicAck( t_envelope );
                continue;
            }

            const param_node* t_msg_payload = t_msg_node->node_at( "payload" );
            if( t_msg_payload == NULL )
            {
                MTERROR( mtlog, "There was no payload present in the message" );
                delete t_msg_node;
                t_connection->amqp()->BasicAck( t_envelope );
                continue;
            }


            switch( t_msg_node->get_value< unsigned >( "msgop", OP_UNKNOWN ) )
            {
                case OP_RUN:
                {
                    MTDEBUG( mtlog, "Run operation request received" );

                    // required
                    const param_node* t_file_node = t_msg_payload->node_at( "file" );
                    if( t_file_node == NULL )
                    {
                        MTERROR( mtlog, "No file configuration present; aborting request" );
                        t_connection->amqp()->BasicAck( t_envelope );
                        break;
                    }

                    // optional
                    const param_node* t_client_node = t_msg_payload->node_at( "client" );

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

                    t_connection->amqp()->BasicAck( t_envelope );
                    t_run_desc->set_status( run_description::acknowledged );

                    MTINFO( mtlog, "Queuing request" );
                    f_run_database->enqueue( t_run_desc );

                    // if the queue condition is waiting, release it
                    if( f_queue_condition->is_waiting() == true )
                    {
                        //MTINFO( mtlog, "releasing queue condition" );
                        f_queue_condition->release();
                    }
                    break;
                }
                case OP_GET:
                {
                    MTDEBUG( mtlog, "Get request received" );

                    std::string t_query_type( t_msg_payload->get_value( "get", "" ) );
                    t_connection->amqp()->BasicAck( t_envelope );

                    if( ! t_envelope->Message()->ReplyToIsSet() )
                    {
                        MTWARN( mtlog, "Query request has no reply-to" );
                        break;
                    }

                    std::string t_reply_to( t_envelope->Message()->ReplyTo() );

                    param_node t_reply;
                    if( t_query_type == "config" )
                    {
                        t_reply.add( "payload", f_master_server_config );
                        t_reply.add( "msgtype", param_value() << T_REPLY );
                    }
                    else if( t_query_type == "mantis" )
                    {
                        param_node* t_msg_node = new param_node();
                        t_msg_node->add( "error", param_value() << "Query type <mantis> is not yet supported" );
                        t_reply.add( "payload", t_msg_node );
                        t_reply.add( "msgtype", param_value() << T_REPLY );
                    }
                    else
                    {
                        param_node* t_msg_node = new param_node();
                        t_msg_node->add( "error", param_value() << "Unrecognized query type or no query type provided" );
                        t_reply.add( "payload", t_msg_node );
                        t_reply.add( "msgtype", param_value() << T_REPLY );
                    }

                    //t_reply.add( "msgop", param_value() << OP_RUN ); // operations aren't used for replies
                    //t_reply.add( "target", param_value() << t_reply_to );  // use of the target is now deprecated (3/12/15)
                    t_reply.add( "timestamp", param_value() << get_absolute_time_string() );

                    std::string t_reply_str;
                    if(! param_output_json::write_string( t_reply, t_reply_str, param_output_json::k_compact ) )
                    {
                        MTERROR( mtlog, "Could not convert reply to string" );
                        break;
                    }

                    AmqpClient::BasicMessage::ptr_t t_reply_msg = AmqpClient::BasicMessage::Create( t_reply_str );
                    t_reply_msg->ContentEncoding( "application/json" );
                    t_reply_msg->CorrelationId( t_envelope->Message()->CorrelationId() );

                    try
                    {
                        t_connection->amqp()->BasicPublish( "", t_envelope->Message()->ReplyTo(), t_reply_msg );
                    }
                    catch( AmqpClient::MessageReturnedException& e )
                    {
                        MTERROR( mtlog, "Reply message could not be sent: " << e.what() );
                    }

                    break;
                }
                case OP_SET:
                {
                    MTDEBUG( mtlog, "Set request received:\n" << *t_msg_payload );

                    std::string t_action( t_msg_payload->get_value( "action", "" ) );
                    const param_node* t_set_node = t_msg_payload->node_at( "set" );
                    if( t_set_node == NULL )
                    {
                        MTWARN( mtlog, "No setting was provided" );
                        break;
                    }

                    if( t_action == "merge" )
                    {
                        f_master_server_config.merge( *t_set_node );
                    }
                    else if( t_action == "replace" )
                    {
                        f_master_server_config = *t_set_node;
                    }

                    t_connection->amqp()->BasicAck( t_envelope );

                    if( ! t_envelope->Message()->ReplyToIsSet() )
                    {
                        MTWARN( mtlog, "Set request has no reply-to" );
                        break;
                    }

                    std::string t_reply_to( t_envelope->Message()->ReplyTo() );

                    param_node t_reply;
                    t_reply.add( "payload", f_master_server_config );
                    t_reply.add( "msgtype", param_value() << T_REPLY );
                    //t_reply.add( "msgop", param_value() << OP_RUN ); // operations aren't used for replies
                    //t_reply.add( "target", param_value() << t_reply_to );  // use of the target is now deprecated (3/12/15)
                    t_reply.add( "timestamp", param_value() << get_absolute_time_string() );

                    std::string t_reply_str;
                    if(! param_output_json::write_string( t_reply, t_reply_str, param_output_json::k_compact ) )
                    {
                        MTERROR( mtlog, "Could not convert reply to string" );
                        break;
                    }

                    AmqpClient::BasicMessage::ptr_t t_reply_msg = AmqpClient::BasicMessage::Create( t_reply_str );
                    t_reply_msg->ContentEncoding( "application/json" );
                    t_reply_msg->CorrelationId( t_envelope->Message()->CorrelationId() );

                    try
                    {
                        t_connection->amqp()->BasicPublish( "", t_envelope->Message()->ReplyTo(), t_reply_msg );
                    }
                    catch( AmqpClient::MessageReturnedException& e )
                    {
                        MTERROR( mtlog, "Reply message could not be sent: " << e.what() );
                    }

                    break;
                }
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
