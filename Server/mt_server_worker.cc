#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_file_writer.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_queue.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"
#include "mt_writer.hh"


namespace mantis
{
    MTLOGGER( mtlog, "server_worker" );

    server_worker::server_worker( const param_node* a_config, digitizer* a_digitizer, buffer* a_buffer, run_queue* a_run_queue, condition* a_queue_condition, condition* a_buffer_condition ) :
            f_config( a_config ),
            f_digitizer( a_digitizer ),
            f_writer( NULL ),
            f_buffer( a_buffer ),
            f_run_queue( a_run_queue ),
            f_queue_condition( a_queue_condition ),
            f_buffer_condition( a_buffer_condition ),
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
            if( f_run_queue->empty() == true )
            {
                // thread cancellation point via call to pthread_cond_wait in queue_condition::wait
                f_queue_condition->wait();
            }

            bool t_communication_fail = false;

            MTINFO( mtlog, "sending server status <started>..." );

            run_context_dist* t_run_context = f_run_queue->from_front();
            t_run_context->lock_status_out()->set_state( status_state_t_started );
            try
            {
                if( ! t_run_context->push_status_no_mutex() )
                {
                    MTERROR( mtlog, "unable to send status <started> to the client; aborting" );
                    t_communication_fail = true;
                }
            }
            catch( closed_connection& cc )
            {
                MTERROR( mtlog, "closed connection detected by: " << cc.what() );
                t_communication_fail = true;
            }
            t_run_context->unlock_outbound();

            if( t_communication_fail )
            {
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            MTINFO( mtlog, "initializing..." );

            request* t_request = t_run_context->lock_request_in();
            f_digitizer->initialize( t_request );

            MTINFO( mtlog, "creating writer..." );

            if(! f_digitizer->write_mode_check( t_request->file_write_mode() ) )
            {
                MTERROR( mtlog, "unable to operate in write mode " << t_request->file_write_mode() << '\n'
                        << "                run request ignored" );
                t_run_context->unlock_inbound();

                t_run_context->lock_status_out()->set_state( status_state_t_error );
                try
                {
                    t_run_context->push_status_no_mutex();
                }
                catch( closed_connection& cc )
                {}
                t_run_context->unlock_outbound();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            factory< writer >* t_writer_factory = factory< writer >::get_instance();
            if( t_request->file_write_mode() == request_file_write_mode_t_local )
            {
                f_writer = t_writer_factory->create( "file" );
                run_description* t_run_desc = new run_description();
                // TODO: extract server executable info from somewhere
                //t_run_desc->set_mantis_server_exe(  );
                t_run_desc->set_mantis_server_version( "Mantis_VERSION" );
                t_run_desc->set_mantis_server_commit( "Mantis_GIT_COMMIT" );
                static_cast< file_writer* >( f_writer )->set_run_description( t_run_desc );
            }
            else
            {
                f_writer = t_writer_factory->create( "network" );
            }
            f_writer->set_buffer( f_buffer, f_buffer_condition );
            try
            {
                f_writer->configure( f_config );
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "unable to configure writer: " << e.what() );
                t_run_context->unlock_inbound();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }
            if( ! f_writer->initialize( t_request ) )
            {
                t_run_context->unlock_inbound();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            t_run_context->unlock_inbound();

            MTINFO( mtlog, "running..." );

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();
            f_digitizer_state = k_running;

            while( f_buffer_condition->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();
            f_writer_state = k_running;

            t_run_context->lock_status_out()->set_state( status_state_t_running );
            try
            {
                if( ! t_run_context->push_status_no_mutex() )
                {
                    MTERROR( mtlog, "unable to send status <running> to the client" );
                    t_communication_fail = true;
                }
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "closed connection detected by: " << cc.what() );
                t_communication_fail = true;
            }
            t_run_context->unlock_outbound();
            if( t_communication_fail )
            {
                MTERROR( mtlog, "; canceling run" );
                f_digitizer->cancel();
                f_writer->cancel();
            }

            // cancellation point (I think) via calls to pthread_join
            t_digitizer_thread->join();
            f_digitizer_state = k_inactive;
            t_writer_thread->join();
            f_writer_state = k_inactive;

            delete t_digitizer_thread;
            delete t_writer_thread;

            if( t_communication_fail )
            {
                delete f_writer;
                f_writer = NULL;
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }


            //t_run_context = f_run_queue->from_front();
            status* t_status = t_run_context->lock_status_out();
            if( ! f_canceled.load() )
            {
                MTINFO( mtlog, "sending server status <stopped>..." );
                t_status->set_state( status_state_t_stopped );
            }
            else
            {
                MTINFO( mtlog, "sending server status <canceled>..." );
                t_status->set_state( status_state_t_canceled );
            }
            try
            {
                if( ! t_run_context->push_status_no_mutex() )
                {
                    MTERROR( mtlog, "unable to send status <stopped>/<canceled> to client" );
                    t_communication_fail = true;
                }
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "closed connection detected by: " << cc.what() );
                t_communication_fail = true;
            }
            t_run_context->unlock_outbound();

            if( t_communication_fail )
            {
                delete f_writer;
                f_writer = NULL;
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            MTINFO( mtlog, "finalizing..." );

            response* t_response = t_run_context->lock_response_out();
            f_digitizer->finalize( t_response );
            f_writer->finalize( t_response );
            t_response->set_state( response_state_t_complete );
            try
            {
                t_run_context->push_response_no_mutex();
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "closed connection detected by: " << cc.what() );
            }
            t_run_context->unlock_outbound();

            delete f_writer;
            f_writer = NULL;
            delete t_run_context->get_connection();
            delete t_run_context;
        }

        return;
    }

    void server_worker::cancel()
    {
        f_canceled.store( true );

        if( f_digitizer_state == k_running )
        {
            f_digitizer->cancel();
        }
        if( f_writer_state == k_running )
        {
            f_writer->cancel();
        }

        return;
    }

    void server_worker::set_writer( writer* a_writer )
    {
        f_writer = a_writer;
    }

}
