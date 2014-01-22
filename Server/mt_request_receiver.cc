#include "mt_request_receiver.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_queue.hh"
#include "mt_server.hh"

#include <cstddef>


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( server* a_server, run_queue* a_run_queue, condition* a_condition ) :
            f_server( a_server ),
            f_run_queue( a_run_queue ),
            f_condition( a_condition ),
            f_buffer_size( 512 ),
            f_record_size( 419304 ),
            f_data_chunk_size( 1024 )
    {
    }

    request_receiver::~request_receiver()
    {
    }

    void request_receiver::execute()
    {
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
                    t_run_context->lock_status_out()->set_state( status_state_t_error );
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
                t_status->set_record_size( f_record_size );
                t_status->set_data_chunk_size( f_data_chunk_size );
                t_run_context->push_status_no_mutex();
                t_run_context->unlock_outbound();

                MTINFO( mtlog, "waiting for client readiness..." );

                if( ! t_run_context->pull_client_status( MSG_WAITALL ) )
                {
                    MTERROR( mtlog, "unable to pull client status; sending server status <error>" );
                    t_run_context->lock_status_out()->set_state( status_state_t_error );
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
                    t_run_context->lock_status_out()->set_state( status_state_t_error );
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

        return;
    }

    void request_receiver::cancel()
    {
        return;
    }

    size_t request_receiver::get_buffer_size() const
    {
        return f_buffer_size;
    }
    void request_receiver::set_buffer_size( size_t size )
    {
        f_buffer_size = size;
        return;
    }

    size_t request_receiver::get_record_size() const
    {
        return f_record_size;
    }
    void request_receiver::set_record_size( size_t size )
    {
        f_record_size = size;
        return;
    }

    size_t request_receiver::get_data_chunk_size() const
    {
        return f_data_chunk_size;
    }
    void request_receiver::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }

}
