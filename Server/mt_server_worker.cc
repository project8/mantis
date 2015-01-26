#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_digitizer.hh"
#include "mt_file_writer.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_database.hh"
#include "mt_run_description.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"

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

            MTINFO( mtlog, "setting run status <started>..." );

            run_description* t_run_desc = f_run_database->pop();
            t_run_desc->set_status( run_description::started );

            MTINFO( mtlog, "initializing..." );

            std::cout << "Retrieved request from the queue:\n" << *t_run_desc << std::endl;

            if( ! f_dev_mgr->configure( *t_run_desc ) )
            {
                MTERROR( mtlog, "unable to configure device manager" );
                t_run_desc->set_status( run_description::error );
                continue;
            }

            MTINFO( mtlog, "creating writer..." );

            file_writer t_writer;
            t_writer.set_device_manager( f_dev_mgr );
            t_writer.set_buffer( f_dev_mgr->get_buffer(), f_dev_mgr->buffer_condition() );

            if( ! t_writer.initialize( t_run_desc ) )
            {
                MTERROR( mtlog, "unable to initialize writer" );
                t_run_desc->set_status( run_description::error );
                continue;
            }

            MTINFO( mtlog, "running..." );
            t_run_desc->set_status( run_description::running );

            f_digitizer = f_dev_mgr->device();
            f_writer = &t_writer;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();
            f_digitizer_state = k_running;

            while( f_dev_mgr->buffer_condition()->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();
            f_writer_state = k_running;


            // cancellation point (I think) via calls to pthread_join
            t_digitizer_thread->join();
            f_digitizer_state = k_inactive;
            t_writer_thread->join();
            f_writer_state = k_inactive;

            f_digitizer = NULL;
            f_writer = NULL;

            delete t_digitizer_thread;
            delete t_writer_thread;

            if( ! f_canceled.load() )
            {
                MTINFO( mtlog, "sending server status <stopped>..." );
                t_run_desc->set_status( run_description::stopped );
            }
            else
            {
                MTINFO( mtlog, "sending server status <canceled>..." );
                t_run_desc->set_status( run_description::canceled );
            }

            MTINFO( mtlog, "finalizing..." );

            response t_response;
            f_dev_mgr->device()->finalize( &t_response );
            t_writer.finalize( &t_response );
            t_response.set_state( response_state_t_complete );
            t_run_desc->set_response_string( t_response.SerializeAsString() );
        }

        return;
    }

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
