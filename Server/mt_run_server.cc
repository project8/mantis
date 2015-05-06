/*
 * mt_run_server.cc
 *
 *  Created on: May 6, 2015
 *      Author: nsoblath
 */

#include "mt_run_server.hh"

#include "mt_condition.hh"
#include "mt_config_manager.hh"
#include "mt_constants.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_request_receiver.hh"
#include "mt_acq_request_db.hh"
#include "mt_server_worker.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"

namespace mantis
{
    MTLOGGER( mtlog, "run_server" );

    run_server::run_server( const param_node& a_node, const std::string& a_exe_name ) :
            f_config( a_node ),
            f_exe_name( a_exe_name ),
            f_return( RETURN_ERROR ),
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

        // device manager
        device_manager t_dev_mgr;

        // configuration manager
        config_manager t_config_mgr( f_config, &t_dev_mgr );

        // run database and queue condition
        condition t_queue_condition;
        acq_request_db t_acq_request_db( &t_config_mgr, &t_queue_condition, f_exe_name );

        // request receiver
        request_receiver t_receiver( &t_config_mgr, &t_acq_request_db );

        // server worker
        server_worker t_worker( &t_dev_mgr, &t_acq_request_db, &t_queue_condition );

        MTINFO( mtlog, "starting threads..." );

        thread t_receiver_thread( &t_receiver );
        thread t_worker_thread( &t_worker );

        signal_handler t_sig_hand;
        t_sig_hand.push_thread( &t_receiver_thread );
        t_sig_hand.push_thread( &t_worker_thread );

        t_receiver_thread.start();
        t_worker_thread.start();

        set_status( k_running );
        MTINFO( mtlog, "running..." );

        t_receiver_thread.join();
        t_worker_thread.join();

        set_status( k_done );

        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread(); // worker thread
            t_sig_hand.pop_thread(); // receiver thread
        }

        MTINFO( mtlog, "Shutting down server" );

        f_return = RETURN_SUCCESS;

        return;
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
