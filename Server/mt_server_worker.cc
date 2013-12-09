#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_queue.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_writer.hh"

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

namespace mantis
{

    server_worker::server_worker( configurator* a_config, digitizer* a_digitizer, buffer* a_buffer, run_queue* a_run_queue, condition* a_queue_condition, condition* a_buffer_condition ) :
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
        while( ! signal_handler::got_exit_signal() )
        {
            if( f_run_queue->empty() == true )
            {
                // thread cancellation point via call to pthread_cond_wait in queue_condition::wait
                f_queue_condition->wait();
            }

            cout << "[server_worker] sending server status <started>..." << endl;

            run_context_dist* t_run_context = f_run_queue->from_front();
            t_run_context->lock_status_out()->set_state( status_state_t_started );
            if( ! t_run_context->push_status_no_mutex() )
            {
                t_run_context->unlock_outbound();
                cerr << "[server_wroker] unable to send status <started> to the client; aborting" << endl;
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }
            t_run_context->unlock_outbound();

            cout << "[server_worker] initializing..." << endl;

            request* t_request = t_run_context->lock_request_in();
            f_digitizer->initialize( t_request );

            cout << "[server_worker] creating writer..." << endl;

            if(! f_digitizer->write_mode_check( t_request->file_write_mode() ) )
            {
                cerr << "[server_worker] unable to operate in write mode " << t_request->file_write_mode() << endl;
                cerr << "                run request ignored" << endl;
                t_run_context->unlock_inbound();

                t_run_context->lock_status_out()->set_state( status_state_t_error );
                t_run_context->push_status_no_mutex();
                t_run_context->unlock_outbound();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            factory< writer >* t_writer_factory = factory< writer >::get_instance();
            if( t_request->file_write_mode() == request_file_write_mode_t_local )
            {
                f_writer = t_writer_factory->create( "file" );
            }
            else
            {
                f_writer = t_writer_factory->create( "network" );
            }
            f_writer->set_buffer( f_buffer, f_buffer_condition );
            f_writer->configure( f_config );
            f_writer->initialize( t_request );

            t_run_context->unlock_inbound();

            cout << "[server_worker] running..." << endl;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );
/*
            signal_handler t_sig_hand;
            t_sig_hand.push_thread( t_writer_thread );
            t_sig_hand.push_thread( t_digitizer_thread );
*/
            t_digitizer_thread->start();
            f_digitizer_state = k_running;

            while( f_buffer_condition->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();
            f_writer_state = k_running;

            t_run_context->lock_status_out()->set_state( status_state_t_running );
            bool t_push_result = t_run_context->push_status_no_mutex();
            t_run_context->unlock_outbound();
            if( ! t_push_result )
            {
                cerr << "[server_wroker] unable to send status <running> to the client; canceling run" << endl;
                cancel();
            }
            //f_run_queue->to_front( f_current_run_context );
            //f_current_run_context = NULL;

            // cancellation point (I think) via calls to pthread_join
            cout << " calling digitizer join" << endl;
            t_digitizer_thread->join();
            f_digitizer_state = k_inactive;
            cout << " calling writer join" << endl;
            t_writer_thread->join();
            f_writer_state = k_inactive;

            cout << "### after thread join ###" << endl;
/*
            if( ! t_sig_hand.got_exit_signal() )
            {
                t_sig_hand.pop_thread(); // digitizer thread
                t_sig_hand.pop_thread(); // writer thread
            }
*/
            delete t_digitizer_thread;
            delete t_writer_thread;


            //t_run_context = f_run_queue->from_front();
            status* t_status = t_run_context->lock_status_out();
            if( ! f_canceled.load() )
            {
                cout << "[server_worker] sending server status <stopped>..." << endl;
                t_status->set_state( status_state_t_stopped );
            }
            else
            {
                cout << "[server_worker] sending server status <canceled>..." << endl;
                t_status->set_state( status_state_t_canceled );
            }
            t_run_context->push_status_no_mutex();
            t_run_context->unlock_outbound();

            cout << "[server_worker] finalizing..." << endl;

            response* t_response = t_run_context->lock_response_out();
            f_digitizer->finalize( t_response );
            f_writer->finalize( t_response );
            t_run_context->push_response_no_mutex();
            t_run_context->unlock_outbound();

            delete t_run_context->get_connection();
            delete t_run_context;
        }

        return;
    }

    void server_worker::cancel()
    {
        std::cout << "CANCELLING SERVER WORKER" << std::endl;
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
