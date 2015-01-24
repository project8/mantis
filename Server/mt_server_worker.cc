#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_file_writer.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_database.hh"
#include "mt_run_description.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"
#include "mt_writer.hh"

#include <sstream>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "server_worker" );

    server_worker::server_worker( const param_node* a_config, digitizer* a_digitizer, buffer* a_buffer, run_database* a_run_database, condition* a_queue_condition, condition* a_buffer_condition ) :
            f_config( a_config ),
            f_digitizer( a_digitizer ),
            f_writer( NULL ),
            f_buffer( a_buffer ),
            f_run_database( a_run_database ),
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
            if( f_run_database->queue_empty() == true )
            {
                // thread cancellation point via call to pthread_cond_wait in queue_condition::wait
                f_queue_condition->wait();
            }

            MTINFO( mtlog, "setting run status <started>..." );

            run_description* t_run_desc = f_run_database->pop();
            t_run_desc->set_status( run_description::started );

            t_run_desc->set_server_config( *f_config );

            MTINFO( mtlog, "initializing..." );

            //TODO for now, just give the request the minimum needed to configure the digitizer
            param_node* t_client_config = t_run_desc->node_at( "client-config" );
            request t_request;
            //t_request.ParseFromString( t_run_desc->get_value( "request-string" ) );
            //t_request.set_write_host( t_write_host );
            //t_request.set_write_port( t_write_port );
            //t_request.set_file( t_client_config->get_value< string >( "file" ) );
            //t_request.set_description( t_client_config->get_value< string >( "description", "default client run" ) );
            t_request.set_date( get_absolute_time_string() );
            t_request.set_mode( (request_mode_t)t_client_config->get_value< int >( "mode" ) );
            t_request.set_rate( t_client_config->get_value< double >( "rate" ) );
            t_request.set_duration( t_client_config->get_value< double >( "duration" ) );
            //t_request.set_file_write_mode( request_file_write_mode_t_local );
            //t_request.set_client_exe( f_exe_name );
            //t_request.set_client_version( TOSTRING(Mantis_VERSION) );
            //t_request.set_client_commit( TOSTRING(Mantis_GIT_COMMIT) );

            f_digitizer->initialize( &t_request );

            MTINFO( mtlog, "creating writer..." );

            // factory still used for writer from when we were potentially writing to different kinds of writers
            factory< writer >* t_writer_factory = factory< writer >::get_instance();
            f_writer = t_writer_factory->create( "file" );

            static_cast< file_writer* >( f_writer )->set_run_description( t_run_desc );

            f_writer->set_buffer( f_buffer, f_buffer_condition );
            try
            {
                f_writer->configure( f_config );
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "unable to configure writer: " << e.what() );
                t_run_desc->set_status( run_description::error );
                continue;
            }
            if( ! f_writer->initialize( &t_request ) )
            {
                t_run_desc->set_status( run_description::error );
                continue;
            }

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

            t_run_desc->set_status( run_description::running );

            // cancellation point (I think) via calls to pthread_join
            t_digitizer_thread->join();
            f_digitizer_state = k_inactive;
            t_writer_thread->join();
            f_writer_state = k_inactive;

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
            f_digitizer->finalize( &t_response );
            f_writer->finalize( &t_response );
            t_response.set_state( response_state_t_complete );
            t_run_desc->set_response_string( t_response.SerializeAsString() );

            delete f_writer;
            f_writer = NULL;
        }

        return;
    }

    void server_worker::cancel()
    {
        MTDEBUG( mtlog, "Canceling server_worker" );
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
