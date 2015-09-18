/*
 * mt_run_server.cc
 *
 *  Created on: May 6, 2015
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_run_server.hh"

#include "mt_amqp_relayer.hh"
#include "mt_broker.hh"
#include "mt_condition.hh"
#include "mt_config_manager.hh"
#include "mt_constants.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_message.hh"
#include "mt_request_receiver.hh"
#include "mt_acq_request_db.hh"
#include "mt_server_worker.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"

#include <signal.h> // for raise()


namespace mantis
{
    MTLOGGER( mtlog, "run_server" );

    run_server::run_server( const param_node& a_node, const version* a_version ) :
            f_config( a_node ),
            f_version( a_version ),
            f_return( RETURN_ERROR ),
            f_request_receiver( NULL ),
            f_server_worker( NULL ),
            f_acq_request_db( NULL ),
            f_component_mutex(),
            f_status( k_initialized )
    {
    }

    run_server::~run_server()
    {
    }

    void run_server::execute()
    {
        MTINFO( mtlog, "Creating server objects" );

        set_status( k_starting );

        const param_node* t_broker_node = f_config.node_at( "amqp" );
        broker t_broker( t_broker_node->get_value( "broker" ), t_broker_node->get_value< unsigned >( "broker-port" ) );
        amqp_channel_ptr t_channel = t_broker.open_channel();
        if( ! t_channel )
        {
            MTERROR( mtlog, "AMQP channel did not open: " << t_broker.get_address() << ":" << t_broker.get_port());
            f_return = RETURN_ERROR;
            return;
        }

        signal_handler t_sig_hand;

        // device manager
        device_manager t_dev_mgr;

        // configuration manager
        config_manager t_config_mgr( f_config, &t_dev_mgr );

        f_component_mutex.lock();

        // run database and queue condition
        acq_request_db t_acq_request_db( &t_config_mgr, f_version->exe_name() );
        f_acq_request_db = &t_acq_request_db;

        // amqp relayer
        amqp_relayer t_amqp_relayer( &t_broker );
        if( ! t_amqp_relayer.initialize( t_broker_node ) )
        {
            MTERROR( mtlog, "Unable to start the AMQP relayer" );
            f_return = R_AMQP_ERROR;
            return;
        }

        thread t_amqp_relayer_thread( &t_amqp_relayer );
        t_sig_hand.push_thread( &t_amqp_relayer_thread );
        t_amqp_relayer_thread.start();

        // server worker
        server_worker t_worker( &t_dev_mgr, &t_acq_request_db, &t_amqp_relayer, t_broker_node );
        f_server_worker = &t_worker;

        // request receiver
        request_receiver t_receiver( this, &t_config_mgr, &t_acq_request_db, &t_worker, t_channel );
        f_request_receiver = &t_receiver;

        f_component_mutex.unlock();

        MTINFO( mtlog, "Starting threads" );

        thread t_receiver_thread( &t_receiver );
        thread t_worker_thread( &t_worker );

        t_sig_hand.push_thread( &t_receiver_thread );
        t_sig_hand.push_thread( &t_worker_thread );

        t_receiver_thread.start();
        t_worker_thread.start();

        set_status( k_running );
        MTINFO( mtlog, "running..." );

        t_receiver_thread.join();
        t_worker_thread.join();

        set_status( k_done );

        f_component_mutex.lock();
        f_request_receiver = NULL;
        f_server_worker = NULL;
        f_acq_request_db = NULL;
        f_component_mutex.unlock();

        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread(); // worker thread
            t_sig_hand.pop_thread(); // receiver thread
            t_sig_hand.pop_thread(); // amqp_relayer thread
            t_amqp_relayer.cancel();
        }

        MTINFO( mtlog, "Threads stopped" );

        f_return = RETURN_SUCCESS;

        return;
    }

    void run_server::quit_server()
    {
        MTINFO( mtlog, "Shutting down the server" );
        raise( SIGINT );
        return;
    }


    bool run_server::handle_get_server_status_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        param_node* t_server_node = new param_node();
        t_server_node->add( "status", new param_value( run_server::interpret_status( get_status() ) ) );

        f_component_mutex.lock();
        if( f_request_receiver != NULL )
        {
            param_node* t_rr_node = new param_node();
            t_rr_node->add( "status", new param_value( request_receiver::interpret_status( f_request_receiver->get_status() ) ) );
            t_server_node->add( "request-receiver", t_rr_node );
        }
        if( f_acq_request_db != NULL )
        {
            param_node* t_queue_node = new param_node();
            t_queue_node->add( "size", new param_value( (uint32_t)f_acq_request_db->queue_size() ) );
            t_queue_node->add( "is-active", new param_value( f_acq_request_db->queue_is_active() ) );
            t_server_node->add( "queue", t_queue_node );
        }
        if( f_server_worker != NULL )
        {
            param_node* t_sw_node = new param_node();
            t_sw_node->add( "status", new param_value( server_worker::interpret_status( f_server_worker->get_status() ) ) );
            t_sw_node->add( "digitizer-state", new param_value( server_worker::interpret_thread_state( f_server_worker->get_digitizer_state() ) ) );
            t_sw_node->add( "writer-state", new param_value( server_worker::interpret_thread_state( f_server_worker->get_writer_state() ) ) );
            t_server_node->add( "server-worker", t_sw_node );
        }
        f_component_mutex.unlock();

        a_pkg.f_payload.add( "server", t_server_node );

        return a_pkg.send_reply( R_SUCCESS, "Server status request succeeded" );
    }

    bool run_server::handle_stop_all_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        param_node* t_server_node = new param_node();
        t_server_node->add( "status", new param_value( run_server::interpret_status( get_status() ) ) );

        f_component_mutex.lock();
        if( f_request_receiver != NULL )
        {
            param_node* t_rr_node = new param_node();
            t_rr_node->add( "status", new param_value( request_receiver::interpret_status( f_request_receiver->get_status() ) ) );
            t_server_node->add( "request-receiver", t_rr_node );
        }
        if( f_acq_request_db != NULL )
        {
            param_node* t_queue_node = new param_node();
            t_queue_node->add( "size", new param_value( (uint32_t)f_acq_request_db->queue_size() ) );
            t_queue_node->add( "is-active", new param_value( f_acq_request_db->queue_is_active() ) );
            t_server_node->add( "queue", t_queue_node );
        }
        if( f_server_worker != NULL )
        {
            param_node* t_sw_node = new param_node();
            t_sw_node->add( "status", new param_value( server_worker::interpret_status( f_server_worker->get_status() ) ) );
            t_sw_node->add( "digitizer-state", new param_value( server_worker::interpret_thread_state( f_server_worker->get_digitizer_state() ) ) );
            t_sw_node->add( "writer-state", new param_value( server_worker::interpret_thread_state( f_server_worker->get_writer_state() ) ) );
            t_server_node->add( "server-worker", t_sw_node );
        }
        f_component_mutex.unlock();

        a_pkg.f_payload.add( "server", t_server_node );

        return a_pkg.send_reply( R_SUCCESS, "Server status request succeeded" );
    }

    bool run_server::handle_quit_server_request( const msg_request* /*a_request*/, request_reply_package& a_pkg )
    {
        bool t_return = a_pkg.send_reply( R_SUCCESS, "Server-quit command processed" );
        quit_server();
        return t_return;
    }


    std::string run_server::interpret_status( status a_status )
    {
        switch( a_status )
        {
            case k_initialized:
                return std::string( "Initialized" );
                break;
            case k_starting:
                return std::string( "Starting" );
                break;
            case k_running:
                return std::string( "Running" );
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


} /* namespace mantis */
