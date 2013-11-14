#include "mt_client_worker.hh"

#include "mt_thread.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    client_worker::client_worker( request* a_request, record_receiver* a_receiver, writer* a_writer, condition* a_buffer_condition ) :
            f_receiver( a_receiver ),
            f_writer( a_writer ),
            f_buffer_condition( a_buffer_condition )
    {
        f_writer->initialize( a_request );
    }

    client_worker::~client_worker()
    {
    }

    void client_worker::execute()
    {
        record_dist* t_record_dist;


        thread* t_receiver_thread = new thread( f_receiver );
        thread* t_writer_thread = new thread( f_writer );

        t_receiver_thread->start();

        while( f_buffer_condition->is_waiting() == false )
        {
            usleep( 1000 );
        }

        t_writer_thread->start();

        cout << "[client_worker] running..." << endl;

        t_receiver_thread->join();
        t_writer_thread->join();

        delete t_receiver_thread;
        delete t_writer_thread;





        while( true )
        {
            if( f_request_queue->empty() == true )
            {
                f_queue_condition->wait();
            }

            cout << "[client_worker] sending status <started>..." << endl;

            t_request_dist = f_request_queue->from_front();
            t_request_dist->get_status()->set_state( status_state_t_started );
            t_request_dist->push_status();

            cout << "[client_worker] initializing..." << endl;

            f_digitizer->initialize( t_request_dist->get_request() );
            f_writer->initialize( t_request_dist->get_request() );

            cout << "[client_worker] running..." << endl;

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

            cout << "[client_worker] sending status <stopped>..." << endl;

            t_request_dist = f_request_queue->from_front();
            t_request_dist->get_status()->set_state( status_state_t_stopped );
            t_request_dist->push_status();

            cout << "[client_worker] finalizing..." << endl;

            f_digitizer->finalize( t_request_dist->get_response() );
            f_writer->finalize( t_request_dist->get_response() );
            t_request_dist->push_response();

            delete t_request_dist->get_connection();
            delete t_request_dist;
        }

        return;
    }

}
