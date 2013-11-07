#include "mt_worker.hh"

#include "mt_thread.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    worker::worker( digitizer* a_digitizer, writer* a_writer, queue* a_queue, condition* a_queue_condition, condition* a_buffer_condition ) :
            f_digitizer( a_digitizer ),
            f_writer( a_writer ),
            f_queue( a_queue ),
            f_queue_condition( a_queue_condition ),
            f_buffer_condition( a_buffer_condition )
    {
    }

    worker::~worker()
    {
    }

    void worker::execute()
    {
        run_context* t_run_context;

        while( true )
        {
            if( f_queue->empty() == true )
            {
                f_queue_condition->wait();
            }

            cout << "[worker] sending status <started>..." << endl;

            t_run_context = f_queue->from_front();
            t_run_context->get_status()->set_state( status_state_t_started );
            t_run_context->push_status();

            cout << "[worker] initializing..." << endl;

            f_digitizer->initialize( t_run_context->get_request() );
            f_writer->initialize( t_run_context->get_request() );

            cout << "[worker] running..." << endl;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();

            while( f_buffer_condition->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();

            t_run_context->get_status()->set_state( status_state_t_running );
            f_queue->to_front( t_run_context );
            t_run_context = NULL;

            t_digitizer_thread->join();
            t_writer_thread->join();

            delete t_digitizer_thread;
            delete t_writer_thread;

            cout << "[worker] sending status <stopped>..." << endl;

            t_run_context = f_queue->from_front();
            t_run_context->get_status()->set_state( status_state_t_stopped );
            t_run_context->push_status();

            cout << "[worker] finalizing..." << endl;

            f_digitizer->finalize( t_run_context->get_response() );
            f_writer->finalize( t_run_context->get_response() );
            t_run_context->push_response();

            delete t_run_context->get_connection();
            delete t_run_context;
        }

        return;
    }

}
