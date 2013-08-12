#include "worker.hh"

#include "thread.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    worker::worker( digitizer* a_digitizer, writer* a_writer, queue* a_queue, condition* a_condition ) :
            f_digitizer( a_digitizer ),
            f_writer( a_writer ),
            f_queue( a_queue ),
            f_condition( a_condition )
    {
    }

    worker::~worker()
    {
    }

    void worker::execute()
    {
        context* t_context;

        while( true )
        {
            if( f_queue->empty() == true )
            {
                f_condition->wait();
            }

            cout << "[worker] sending status <started>..." << endl;

            t_context = f_queue->from_front();
            t_context->get_status()->set_state( status_state_t_started );
            t_context->push_status();

            cout << "[worker] initializing..." << endl;

            f_digitizer->initialize( t_context->get_request() );
            f_writer->initialize( t_context->get_request() );

            cout << "[worker] running..." << endl;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();
            t_writer_thread->start();

            t_context->get_status()->set_state( status_state_t_running );
            f_queue->to_front( t_context );
            t_context = NULL;

            t_digitizer_thread->join();
            t_writer_thread->join();

            delete t_digitizer_thread;
            delete t_writer_thread;

            cout << "[worker] sending status <stopped>..." << endl;

            t_context = f_queue->from_front();
            t_context->get_status()->set_state( status_state_t_stopped );
            t_context->push_status();

            cout << "[worker] finalizing..." << endl;

            f_digitizer->finalize( t_context->get_response() );
            f_writer->finalize( t_context->get_response() );
            t_context->push_response();

            delete t_context->get_connection();
            delete t_context;
        }

        return;
    }

}
