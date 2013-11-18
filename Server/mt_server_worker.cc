#include "mt_server_worker.hh"

#include "mt_thread.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    server_worker::server_worker( digitizer* a_digitizer, writer* a_writer, request_queue* a_request_queue, condition* a_queue_condition, condition* a_buffer_condition ) :
            f_digitizer( a_digitizer ),
            f_writer( a_writer ),
            f_request_queue( a_request_queue ),
            f_queue_condition( a_queue_condition ),
            f_buffer_condition( a_buffer_condition )
    {
    }

    server_worker::~server_worker()
    {
    }

    void server_worker::execute()
    {
        request_dist* t_request_dist;

        while( true )
        {
            if( f_request_queue->empty() == true )
            {
                f_queue_condition->wait();
            }

            cout << "[server_worker] sending server status <started>..." << endl;

            t_request_dist = f_request_queue->from_front();
            t_request_dist->get_status()->set_state( status_state_t_started );
            t_request_dist->push_status();

            cout << "[server_worker] initializing..." << endl;

            f_digitizer->initialize( t_request_dist->get_request() );
            f_writer->initialize( t_request_dist->get_request() );

            cout << "[server_worker] running..." << endl;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();

            while( f_buffer_condition->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();

            t_request_dist->get_status()->set_state( status_state_t_running );
            f_request_queue->to_front( t_request_dist );
            t_request_dist = NULL;

            t_digitizer_thread->join();
            t_writer_thread->join();

            delete t_digitizer_thread;
            delete t_writer_thread;

            cout << "[server_worker] sending server status <stopped>..." << endl;

            t_request_dist = f_request_queue->from_front();
            t_request_dist->get_status()->set_state( status_state_t_stopped );
            t_request_dist->push_status();

            cout << "[server_worker] finalizing..." << endl;

            f_digitizer->finalize( t_request_dist->get_response() );
            f_writer->finalize( t_request_dist->get_response() );
            t_request_dist->push_response();

            delete t_request_dist->get_connection();
            delete t_request_dist;
        }

        return;
    }

}
