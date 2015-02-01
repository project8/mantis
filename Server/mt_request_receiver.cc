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
        connection* t_connection = f_broker->create_connection();
        if( t_connection == NULL )
        {
            MTERROR( mtlog, "Cannot create connection to AMQP broker" );
            cancel();
            raise( SIGINT );
            return;
        }

        t_connection->amqp()->DeclareQueue( "mantis", false, false, true, false );
        t_connection->amqp()->BindQueue( "mantis", "requests", "mantis" );

        std::string t_consumer_tag = t_connection->amqp()->BasicConsume( "mantis", "mantis", true, false ); // second bool is setting no_ack to false

        while( true )
        {
            if( f_canceled.load() ) return;

            // blocking call to wait for incoming message
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
                continue;
            }

            switch( t_msg_node->get_value< unsigned >( "msgop" ) )
            {
                case OP_MANTIS_RUN:
                {
                    MTDEBUG( mtlog, "Run operation request received" );
                    const param_node* t_msg_payload = t_msg_node->node_at( "payload" );

                    // required
                    const param_node* t_file_node = t_msg_payload->node_at( "file" );
                    if( t_file_node == NULL )
                    {
                        MTERROR( mtlog, "No file configuration present; aborting request" );
                        continue;
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
                case OP_MANTIS_QUERY:
                {
                    MTDEBUG( mtlog, "Query request received" );
                    const param_node* t_msg_payload = t_msg_node->node_at( "payload" );

                    std::string t_query_type( t_msg_payload->get_value( "query", "" ) );
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
                        t_reply.add( "msgtype", param_value() << T_MANTIS_REPLY );
                    }
                    else if( t_query_type == "mantis" )
                    {
                        param_node* t_msg_node = new param_node();
                        t_msg_node->add( "error", param_value() << "Query type <mantis> is not yet supported" );
                        t_reply.add( "payload", t_msg_node );
                        t_reply.add( "msgtype", param_value() << T_MANTIS_REPLY );
                    }
                    else
                    {
                        param_node* t_msg_node = new param_node();
                        t_msg_node->add( "error", param_value() << "Unrecognized query type or no query type provided" );
                        t_reply.add( "payload", t_msg_node );
                        t_reply.add( "msgtype", param_value() << T_MANTIS_REPLY );
                    }

                    //t_reply.add( "msgop", param_value() << OP_MANTIS_RUN );
                    t_reply.add( "target", param_value() << t_reply_to );
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
                case OP_MANTIS_CONFIG:
                {
                    MTDEBUG( mtlog, "Config request received" );
                    const param_node* t_msg_payload = t_msg_node->node_at( "payload" );

                    std::string t_action( t_msg_payload->get_value( "action", "" ) );
                    const param_node* t_config_node = t_msg_payload->node_at( "config" );

                    if( t_action == "merge" )
                    {
                        f_master_server_config.merge( *t_config_node );
                    }
                    else if( t_action == "replace" )
                    {
                        f_master_server_config = *t_config_node;
                    }

                    t_connection->amqp()->BasicAck( t_envelope );

                    if( ! t_envelope->Message()->ReplyToIsSet() )
                    {
                        MTWARN( mtlog, "Config request has no reply-to" );
                        break;
                    }

                    std::string t_reply_to( t_envelope->Message()->ReplyTo() );

                    param_node t_reply;
                    t_reply.add( "payload", f_master_server_config );
                    t_reply.add( "msgtype", param_value() << T_MANTIS_REPLY );
                    //t_reply.add( "msgop", param_value() << OP_MANTIS_RUN );
                    t_reply.add( "target", param_value() << t_reply_to );
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
            }
            delete t_msg_node;
        }

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
