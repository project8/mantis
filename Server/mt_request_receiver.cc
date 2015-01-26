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

#include "MonarchVersion.hpp"

#include <cstddef>
#include <signal.h>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( const param_node& a_config, broker* a_broker, run_database* a_run_database, condition* a_queue_condition, const string& a_exe_name ) :
            f_master_server_config( *a_config ),
            f_broker( a_broker ),
            f_run_database( a_run_database ),
            f_queue_condition( a_queue_condition ),
            f_exe_name( a_exe_name )
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
            kill( 0, SIGINT );
            return;
        }

        t_connection->amqp()->DeclareQueue( "mantis", false, false, true, false );
        t_connection->amqp()->BindQueue( "mantis", "requests", "mantis" );

        std::string t_consumer_tag = t_connection->amqp()->BasicConsume( "mantis" );

        while( true )
        {
            // blocking call to wait for incoming message
            AmqpClient::Envelope::ptr_t t_envelope = t_connection->amqp()->BasicConsumeMessage( t_consumer_tag );


            param_node* t_msg_node = NULL;
            if( t_envelope->Message()->ContentEncoding() == "application/json" )
            {
                t_msg_node = param_input_json::read_string( t_envelope->Message()->Body() );
            }
            else
            {
                MTERROR( mtlog, "Unable to parse message with content type <" << t_envelope->Message()->ContentEncoding() << ">" );
            }

            switch( t_msg_node->get_value< unsigned >( "msgop" ) )
            {
                case OP_MANTIS_RUN:
                {
                    MTDEBUG( mtlog, "Run operation request received" );
                    param_node* t_msg_payload = t_msg_node->node_at( "payload" );
                    run_description* t_run_desc = new run_description();
                    t_run_desc->set_status( run_description::created );

                    t_run_desc->set_mantis_server_commit( TOSTRING(Mantis_GIT_COMMIT) );
                    t_run_desc->set_mantis_server_exe( f_exe_name );
                    t_run_desc->set_mantis_server_version( TOSTRING(Mantis_VERSION) );
                    t_run_desc->set_monarch_commit( TOSTRING(Monarch_GIT_COMMIT) );
                    t_run_desc->set_monarch_version( TOSTRING(Monarch_VERSION ) );

                    t_run_desc->set_mantis_config( f_master_server_config );

                    try
                    {
                        t_run_desc->set_file_config( *( t_msg_payload->node_at( "run" )->node_at( "config" ) ) );
                    }
                    catch(...)
                    {
                        MTERROR( mtlog, "No client configuration present; aborting request" );
                    }
                    t_run_desc->set_client_commit( t_msg_payload->node_at( "client" )->get_value( "commit", "N/A" ) );
                    t_run_desc->set_client_exe( t_msg_payload->node_at( "client" )->get_value( "exe", "N/A" ) );
                    t_run_desc->set_client_version( t_msg_payload->node_at( "client" )->get_value( "version", "N/A" ) );
                    t_run_desc->set_description( t_msg_payload->node_at( "run" )->get_value( "description", "N/A" ) );

                    // TODO send acknowledgment
                    t_run_desc->set_status( run_description::acknowledged );

                    MTINFO( mtlog, "queuing request..." );
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
                    MTWARN( mtlog, "Query operations not yet supported; request ignored" );
                    break;
                case OP_MANTIS_CONFIG:
                    MTWARN( mtlog, "Config operations not yet supported; request ignored" );
                    break;
                default:
                    MTERROR( mtlog, "Unrecognized message operation: <" << t_msg_node->get_value< unsigned >( "msgop" ) << ">" );
                    break;
            }
        }


/*

        run_context_dist* t_run_context;

        while( true )
        {
            t_run_context = new run_context_dist();
            MTINFO( mtlog, "waiting for incoming connections" );
            // thread is blocked by the accept call in server::get_connection 
            // until an incoming connection is received
            t_run_context->set_connection( f_server->get_connection() );

            MTINFO( mtlog, "receiving request..." );

            try
            {
                // use blocking option for pull request
                if( ! t_run_context->pull_request( MSG_WAITALL ) )
                {
                    MTERROR( mtlog, "unable to pull run request; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "unable to pull run request" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }

                // check version of client
                // major and minor versions must match
                unsigned t_server_major_ver = Mantis_VERSION_MAJOR;
                unsigned t_server_minor_ver = Mantis_VERSION_MINOR;
                //MTDEBUG( mtlog, "server major ver: " << t_server_major_ver << "; minor ver: " << t_server_minor_ver );
                request* t_request = t_run_context->lock_request_in();
                t_run_context->unlock_inbound();
                version t_client_version( t_request->client_version() );
                unsigned t_client_major_ver = t_client_version.major_version();
                unsigned t_client_minor_ver = t_client_version.minor_version();
                //MTDEBUG( mtlog, "client major ver: " << t_client_major_ver << "; minor ver: " << t_client_minor_ver );

                if( t_server_major_ver != t_client_major_ver || t_server_minor_ver != t_client_minor_ver )
                {
                    MTERROR( mtlog, "client and server software versions do not match:\n" <<
                            "\tServer: " << TOSTRING(Mantis_VERSION) << '\n' <<
                            "\tClient: " << t_client_version.version_str());
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    std::stringstream t_error_msg;
                    t_error_msg << "client (" << t_client_version.version_str() << ") and server (" << TOSTRING(Mantis_VERSION) << ") software versions do not match";
                    t_status->set_error_message( t_error_msg.str() );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }

                MTINFO( mtlog, "sending server status <acknowledged>..." );

                status* t_status = t_run_context->lock_status_out();
                t_status->set_state( status_state_t_acknowledged );
                t_status->set_buffer_size( f_buffer_size );
                t_status->set_record_size( f_block_size );
                t_status->set_data_chunk_size( f_data_chunk_size );
                t_status->set_data_type_size( f_data_type_size );
                t_status->set_bit_depth( f_bit_depth );
                t_status->set_voltage_min( f_voltage_min );
                t_status->set_voltage_range( f_voltage_range );
                t_status->set_server_exe( f_exe_name );
                t_status->set_server_version( "Mantis_VERSION" );
                t_status->set_server_commit( "Mantis_GIT_COMMIT" );
                string t_config_as_string;
                param_output_json::write_string( f_config, t_config_as_string, param_output_json::k_compact );
                t_status->set_server_config( t_config_as_string );

                //t_run_context->push_status_no_mutex();
                t_run_context->unlock_outbound();

                unsigned t_timeout_sec = 5;
                if( ! t_run_context->set_pull_timeout( t_timeout_sec ) )
                {
                    MTWARN( mtlog, "unable to set pull timeout" );
                }
                unsigned t_push_status_attempts = 1;
                unsigned t_max_push_status_attempts = 3;
                int t_ret_errno = 0;
                bool t_ret_val = false, t_try_again = true;
                while( t_try_again )
                {
                    t_run_context->push_status();
                    ++t_push_status_attempts;

                    MTINFO( mtlog, "waiting for client readiness..." );

                    t_ret_val = t_run_context->pull_client_status( MSG_WAITALL, t_ret_errno );
                    if( ! t_ret_val &&
                            t_push_status_attempts < t_max_push_status_attempts &&
                            (t_ret_errno == EWOULDBLOCK || t_ret_errno == EAGAIN) )
                    {
                        t_try_again = true;
                    }
                    else
                    {
                        t_try_again = false;
                    }
                }
                // reset the timeout
                if( ! t_run_context->set_pull_timeout( 0 ) )
                {
                    MTWARN( mtlog, "unable to reset pull timeout" )
                }
                if( ! t_ret_val )
                {
                    MTERROR( mtlog, "unable to pull client status; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "unable to pull client status" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }
                client_status_state_t t_client_state = t_run_context->lock_client_status_in()->state();
                t_run_context->unlock_inbound();
                if( ! t_client_state == client_status_state_t_ready )
                {
                    MTERROR( mtlog, "client did not get ready; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "client is not ready" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "connection closed; detected in <" << cc.what() << ">" );
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            MTINFO( mtlog, "queuing request..." );

            t_run_context->lock_status_out()->set_state( status_state_t_waiting );
            t_run_context->unlock_outbound();
            f_run_queue->to_back( t_run_context );


            // if the queue condition is waiting, release it
            if( f_condition->is_waiting() == true )
            {
                //MTINFO( mtlog, "releasing queue condition" );
                f_condition->release();
            }

            //MTINFO( mtlog, "finished processing request" );
        }
*/
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
        return;
    }

}
