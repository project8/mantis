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

        cout << "[client_worker] finalizing..." << endl;

        response* t_response = new response();
        //f_receiver->finalize( t_response );
        f_writer->finalize( t_response );
        //TODO: do something with the response
        //t_request_dist->push_response();

         return;
    }

}
