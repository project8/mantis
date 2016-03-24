#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS
#define DRIPLINE_API_EXPORTS

#include "mt_server_worker.hh"

#include "mt_amqp_relayer.hh"
#include "mt_acq_request_db.hh"
#include "mt_acq_request.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_digitizer.hh"
#include "mt_file_writer.hh"
#include "logger.hh"
#include "mt_request_receiver.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"

#ifndef _WIN32
#include <unistd.h>
#endif
#include <sstream>

using std::string;

namespace mantis
{
    using dripline::retcode_t;

    LOGGER( mtlog, "server_worker" );

    server_worker::server_worker( device_manager* a_dev_mgr, acq_request_db* a_run_db, amqp_relayer* a_amqp_relayer, const param_node* a_amqp_node ) :
            f_dev_mgr( a_dev_mgr ),
            f_acq_request_db( a_run_db ),
            f_digitizer( NULL ),
            f_writer( NULL ),
            f_component_mutex(),
            f_amqp_relayer( a_amqp_relayer ),
            f_completed_file_key( a_amqp_node->get_value( "completed-file-key", "" ) ),
            f_canceled( false ),
            f_digitizer_state( k_inactive ),
            f_writer_state( k_inactive ),
            f_status( k_initialized )
    {
    }

    server_worker::~server_worker()
    {
    }

    void server_worker::execute()
    {
        while( ! f_canceled.load() )
        {
            f_status.store( k_idle );

            // continue if/when there's a request in the queue
            // this is a thread cancellation point if the queue is empty
            f_acq_request_db->wait_for_request_in_queue();

            // continue if/when the queue is active
            // this is a thread cancellation point if the queue is _not_ active
            f_acq_request_db->wait_for_queue_active();

            f_status.store( k_starting );

            LINFO( mtlog, "Processing run request from queue" );

            LDEBUG( mtlog, "Setting run status <started>" );

            acq_request* t_acq_req = f_acq_request_db->pop();
            t_acq_req->set_status( acq_request::started );

            LINFO( mtlog, "Initializing" );

            LDEBUG( mtlog, "Retrieved request from the queue:\n" << *t_acq_req );

            if( ! f_dev_mgr->configure( *t_acq_req ) )
            {
                LERROR( mtlog, "Unable to configure device manager" );
                t_acq_req->set_status( acq_request::error );
                continue;
            }

            LINFO( mtlog, "Creating writer" );

            file_writer t_writer;
            t_writer.set_device_manager( f_dev_mgr );
            t_writer.set_buffer( f_dev_mgr->get_buffer(), f_dev_mgr->buffer_condition() );

            if( ! t_writer.initialize( t_acq_req ) )
            {
                LERROR( mtlog, "Unable to initialize writer" );
                t_acq_req->set_status( acq_request::error );
                continue;
            }

            LINFO( mtlog, "Setting run status <running>" );
            t_acq_req->set_status( acq_request::running );

            f_component_mutex.lock();
            f_digitizer = f_dev_mgr->device();
            f_writer = &t_writer;
            f_component_mutex.unlock();

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            if( ! f_canceled.load() )
            {
                t_digitizer_thread->start();
                f_digitizer_state = k_running;

                while (f_dev_mgr->buffer_condition()->is_waiting() == false)
                {
#ifndef _WIN32
                    usleep( 1000 );
#else
                    Sleep(1);
#endif
                }

                t_writer_thread->start();
                f_writer_state = k_running;

                f_status.store( k_acquiring );

                // cancellation point (I think) via calls to pthread_join
                t_digitizer_thread->join();
                f_digitizer_state = k_inactive;
                t_writer_thread->join();
                f_writer_state = k_inactive;
            }

            if( f_digitizer->get_status() != digitizer::k_ok )
            {
                LWARN( mtlog, "Digitizer finished in non-ok state: (" << f_digitizer->get_status() << ") " << f_digitizer->get_status_message() );
                LWARN( mtlog, "Processing of requests has been stopped to allow the problem to be fixed" );
                f_acq_request_db->stop_queue();
            }
            if( f_writer->get_status() != writer::k_ok )
            {
                LWARN( mtlog, "Writer finished in non-ok state: (" << f_writer->get_status() << ") " << f_writer->get_status_message() );
                LWARN( mtlog, "Processing of requests has been stopped to allow the problem to be fixed" );
                f_acq_request_db->stop_queue();
            }

            f_component_mutex.lock();
            f_digitizer = NULL;
            f_writer = NULL;
            f_component_mutex.unlock();

            delete t_digitizer_thread;
            delete t_writer_thread;

            if( ! f_canceled.load() )
            {
                f_status.store( k_acquired );

                LINFO( mtlog, "Setting run status <stopped>" );
                t_acq_req->set_status( acq_request::stopped );
            }
            else
            {
                LINFO( mtlog, "Setting run status <canceled>" );
                t_acq_req->set_status( acq_request::canceled );
            }

            LINFO( mtlog, "Finalizing..." );

            param_node t_response;
            f_dev_mgr->device()->finalize( &t_response );
            t_writer.finalize( &t_response );
            t_acq_req->set_response( t_response );
            t_acq_req->set_status( acq_request::stopped );
            LINFO( mtlog, "Run response:\n" << t_response );
            if( ! f_completed_file_key.empty() )
            {
                f_amqp_relayer->send( dripline::msg_alert::create( new param_node( *t_acq_req ), f_completed_file_key, dripline::message::encoding::json ) );
            }
        }

        return;
    }

    /*
    Asyncronous stop-acquisition:
    - The execute function waits while the digitizer and writer run; if stopped, those threads are cancelled, which
    eventually returns control to the execute function, which completes quickly.
    - If cancelled before the threads are started, operation of the digitizer and writer will be skipped.
    */
    void server_worker::stop_acquisition()
    {
        LDEBUG( mtlog, "Stopping acquisition" );
        f_component_mutex.lock();
        if( f_digitizer_state == k_running && f_digitizer != NULL )
        {
            f_digitizer->cancel();
        }
        if( f_writer_state == k_running && f_writer != NULL )
        {
            f_writer->cancel();
        }
        f_component_mutex.unlock();

        return;
    }



    /*
    Asyncronous cancellation:
    - The execute function waits while the digitizer and writer run; if cancelled, those threads are cancelled, which 
    eventually returns control to the execute function, which completes quickly.
    - If cancelled before the threads are started, operation of the digitizer and writer will be skipped.
    */
    void server_worker::cancel()
    {
        LDEBUG( mtlog, "Canceling server_worker" );
        f_canceled.store( true );
        f_status.store( k_canceled );

        stop_acquisition();

        return;
    }

    bool server_worker::handle_stop_acq_request( const request_ptr_t, hub::reply_package& a_reply_pkg )
    {
        stop_acquisition();
        return a_reply_pkg.send_reply( retcode_t::success, "Stop-acquisition request succeeded" );
    }


    std::string server_worker::interpret_thread_state( thread_state a_thread_state )
    {
        switch( a_thread_state )
        {
            case k_inactive:
                return std::string( "Inactive" );
                break;
            case k_running:
                return std::string( "Running" );
                break;
            default:
                return std::string( "Unknown" );
        }
    }

    std::string server_worker::interpret_status( status a_status )
    {
        switch( a_status )
        {
            case k_initialized:
                return std::string( "Initialized" );
                break;
            case k_starting:
                return std::string( "Starting" );
                break;
            case k_idle:
                return std::string( "Idle (queue is empty)" );
                break;
            case k_acquiring:
                return std::string( "Acquisition in progress" );
                break;
            case k_acquired:
                return std::string( "Acquisition complete" );
                break;
            case k_canceled:
                return std::string( "Canceled" );
                break;
            case k_error:
                return std::string( "Error" );
                break;
            default:
                return std::string( "Unknown" );
        }
    }


}
