#define MANTIS_API_EXPORTS

#include "mt_request_receiver.hh"

#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_buffer.hh"
#include "mt_config_manager.hh"
#include "mt_api.hh"
#include "mt_exception.hh"
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

    using dripline::retcode_t;

    LOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db, server_worker* a_server_worker ) :
            callable(),
            hub(),
            f_listen_timeout_ms( 100 ),
            f_run_server( a_run_server ),
            f_conf_mgr( a_conf_mgr ),
            f_acq_request_db( a_acq_request_db ),
            f_server_worker( a_server_worker ),
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
                LDEBUG( mtlog, "Canceling consume of tag <" << f_consumer_tag << ">" );
                f_broker->get_connection().amqp()->BasicCancel( f_consumer_tag );
                f_consumer_tag.clear();
            }
            if( ! f_queue_name.empty() )
            {
                LDEBUG( mtlog, "Deleting queue <" << f_queue_name << ">" );
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
        param_node* t_broker_node = f_conf_mgr->copy_master_server_config( "amqp" );

        if( ! dripline_setup( t_broker_node->get_value( "broker" ),
                              t_broker_node->get_value< unsigned >( "broker-port" ),
                              t_broker_node->get_value( "exchange" ),
                              t_broker_node->get_value( "queue" ),
                              ".project8_authentications.json" ) )
        {
            LERROR( mtlog, "Unable to complete dripline setup" );
            f_run_server->quit_server();
            return;
        }

        delete t_broker_node;

        start();

        LINFO( mtlog, "Waiting for incoming messages" );

        f_status.store( k_listening );

        while( ! f_canceled.load() )
        {
            // blocking call to wait for incoming message
            listen( f_listen_timeout_ms );
        }

        LINFO( mtlog, "No longer waiting for messages" );

        stop();

        f_status.store( k_done );
        LDEBUG( mtlog, "Request receiver is done" );

        return;
    }

    bool request_receiver::do_run_request( const request_ptr_t a_request, reply_package& a_reply_pkg )
    {
        return f_acq_request_db->handle_new_acq_request( a_request, a_reply_pkg );
    }

    bool request_receiver::do_get_request( const request_ptr_t a_request, reply_package& a_reply_pkg )
    {
        std::string t_query_type = a_request->parsed_rks().front();
        a_request->parsed_rks().pop_front();

        param_node t_reply;
        if( t_query_type == "acq-config" )
        {
            return f_conf_mgr->handle_get_acq_config_request( a_request, a_reply_pkg );
        }
        else if( t_query_type == "server-config" )
        {
            return f_conf_mgr->handle_get_server_config_request( a_request, a_reply_pkg );
        }
        else if( t_query_type == "acq-status" )
        {
            return f_acq_request_db->handle_get_acq_status_request( a_request, a_reply_pkg );
        }
        else if( t_query_type == "queue" )
        {
            return f_acq_request_db->handle_queue_request( a_request, a_reply_pkg );
        }
        else if( t_query_type == "queue-size" )
        {
            return f_acq_request_db->handle_queue_size_request( a_request, a_reply_pkg );
        }
        else if( t_query_type == "server-status" )
        {
            return f_run_server->handle_get_server_status_request( a_request, a_reply_pkg );
        }
        else
        {
            return a_reply_pkg.send_reply( retcode_t::message_error_bad_payload, "Unrecognized query type or no query type provided" );;
        }
    }

    bool request_receiver::do_set_request( const request_ptr_t a_request, reply_package& a_reply_pkg )
    {
        return f_conf_mgr->handle_set_request( a_request, a_reply_pkg );
    }

    bool request_receiver::do_cmd_request( const request_ptr_t a_request, reply_package& a_reply_pkg )
    {
        LDEBUG( mtlog, "Cmd request received" );

        // get the instruction before checking the lockout key authentication because we need to have the exception for
        // the unlock instruction that allows us to force the unlock.
        std::string t_instruction = a_request->parsed_rks().front();
        a_request->parsed_rks().pop_front();

        if( t_instruction == "replace-config" )
        {
            return f_conf_mgr->handle_replace_acq_config( a_request, a_reply_pkg );
        }
        else if( t_instruction == "add" )
        {
            return f_conf_mgr->handle_add_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "remove" )
        {
            return f_conf_mgr->handle_remove_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "cancel-acq" )
        {
            return f_acq_request_db->handle_cancel_acq_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "clear-queue" )
        {
            return f_acq_request_db->handle_clear_queue_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "start-queue" )
        {
            return f_acq_request_db->handle_start_queue_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "stop-queue" )
        {
            return f_acq_request_db->handle_stop_queue_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "stop-acq" )
        {
            return f_server_worker->handle_stop_acq_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "stop-all" )
        {
            return f_run_server->handle_stop_all_request( a_request, a_reply_pkg );
        }
        else if( t_instruction == "quit-mantis" )
        {
            return f_run_server->handle_quit_server_request( a_request, a_reply_pkg );
        }
        else
        {
            LWARN( mtlog, "Instruction <" << t_instruction << "> not understood" );
            return a_reply_pkg.send_reply( retcode_t::message_error_bad_payload, "Instruction <" + t_instruction + "> not understood" );;
        }
    }



    void request_receiver::cancel()
    {
        LDEBUG( mtlog, "Canceling request receiver" );
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
            case k_listening:
                return std::string( "Listening" );
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
