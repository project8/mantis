#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS

#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_digitizer.hh"
#include "mt_file_writer.hh"
#include "mt_logger.hh"
#include "mt_run_database.hh"
#include "mt_run_description.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <sstream>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "server_worker" );

    server_worker::server_worker( device_manager* a_dev_mgr, run_database* a_run_db, condition* a_queue_condition ) :
            f_dev_mgr( a_dev_mgr ),
            f_run_database( a_run_db ),
            f_queue_condition( a_queue_condition ),
            f_digitizer( NULL ),
            f_writer( NULL ),
            f_canceled( false ),
            f_digitizer_state( k_inactive ),
            f_writer_state( k_inactive )
    {
    }

    server_worker::~server_worker()
    {
    }

    void server_worker::execute()
    {
        while( ! f_canceled.load() )
        {
            if( f_run_database->queue_empty() == true )
            {
                // thread cancellation point via call to pthread_cond_wait in queue_condition::wait
                f_queue_condition->wait();
            }

            MTINFO( mtlog, "Processing run request from queue" );

            MTINFO( mtlog, "Setting run status <started>" );

            run_description* t_run_desc = f_run_database->pop();
            t_run_desc->set_status( run_description::started );

            MTINFO( mtlog, "Initializing" );

            MTDEBUG( mtlog, "Retrieved request from the queue:\n" << *t_run_desc );

            if( ! f_dev_mgr->configure( *t_run_desc ) )
            {
                MTERROR( mtlog, "Unable to configure device manager" );
                t_run_desc->set_status( run_description::error );
                continue;
            }

            MTINFO( mtlog, "Creating writer" );

            file_writer t_writer;
            t_writer.set_device_manager( f_dev_mgr );
            t_writer.set_buffer( f_dev_mgr->get_buffer(), f_dev_mgr->buffer_condition() );

            if( ! t_writer.initialize( t_run_desc ) )
            {
                MTERROR( mtlog, "Unable to initialize writer" );
                t_run_desc->set_status( run_description::error );
                continue;
            }

            MTINFO( mtlog, "Setting run status <running>" );
            t_run_desc->set_status( run_description::running );

            f_digitizer = f_dev_mgr->device();
            f_writer = &t_writer;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            if (!f_canceled.load())
            {
                f_digitizer_state = k_running;
                f_writer_state = k_running;

                t_digitizer_thread->start();

                while (f_dev_mgr->buffer_condition()->is_waiting() == false)
                {
#ifndef _WIN32
                    usleep( 1000 );
#else
                    Sleep(1);
#endif
                }

                t_writer_thread->start();

                // cancellation point (I think) via calls to pthread_join
                t_digitizer_thread->join();
                f_digitizer_state = k_inactive;
                t_writer_thread->join();
                f_writer_state = k_inactive;
            }

            f_digitizer = NULL;
            f_writer = NULL;

            delete t_digitizer_thread;
            delete t_writer_thread;

            if( ! f_canceled.load() )
            {
                MTINFO( mtlog, "Setting run status <stopped>" );
                t_run_desc->set_status( run_description::stopped );
            }
            else
            {
                MTINFO( mtlog, "Setting run status <canceled>" );
                t_run_desc->set_status( run_description::canceled );
            }

            MTINFO( mtlog, "Finalizing..." );

            param_node t_response;
            f_dev_mgr->device()->finalize( &t_response );
            t_writer.finalize( &t_response );
            t_run_desc->set_response( t_response );
            t_run_desc->set_status( run_description::stopped );
            MTINFO( mtlog, "Run response:\n" << t_response );
        }

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
        MTDEBUG( mtlog, "Canceling server_worker" );
        f_canceled.store( true );

        if( f_digitizer_state == k_running && f_digitizer != NULL )
        {
            f_digitizer->cancel();
        }
        if( f_writer_state == k_running && f_writer != NULL )
        {
            f_writer->cancel();
        }

        return;
    }

}
