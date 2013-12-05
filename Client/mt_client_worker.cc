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
            f_buffer_condition( a_buffer_condition ),
            f_receiver_state( k_inactive ),
            f_writer_state( k_running )
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
        f_receiver_state = k_running;

        while( f_buffer_condition->is_waiting() == false )
        {
            usleep( 1000 );
        }

        t_writer_thread->start();
        f_writer_state = k_running;

        cout << "[client_worker] running..." << endl;

        t_receiver_thread->join();
        f_receiver_state = k_inactive;
        t_writer_thread->join();
        f_writer_state = k_inactive;

        delete t_receiver_thread;
        delete t_writer_thread;

        cout << "[client_worker] finalizing..." << endl;

        response t_response;
        f_receiver->finalize( &t_response );
        f_writer->finalize( &t_response );
        cout << "[client_worker] record_receiver summary:\n";
        cout << "  record count: " << t_response.digitizer_records() << " [#]\n";
        cout << "  acquisition count: " << t_response.digitizer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_response.digitizer_live_time() << " [sec]\n";
        cout << "  dead time: " << t_response.digitizer_dead_time() << " [sec]\n";
        cout << "  megabytes: " << t_response.digitizer_megabytes() << " [Mb]\n";
        cout << "  rate: " << t_response.digitizer_rate() << " [Mb/sec]\n";

        cout << endl;

        cout << "[client_worker] file_writer summary:\n";
        cout << "  record count: " << t_response.writer_records() << " [#]\n";
        cout << "  acquisition count: " << t_response.writer_acquisitions() << " [#]\n";
        cout << "  live time: " << t_response.writer_live_time() << " [sec]\n";
        cout << "  megabytes: " << t_response.writer_megabytes() << "[Mb]\n";
        cout << "  rate: " << t_response.writer_rate() << " [Mb/sec]\n";

        cout << endl;

         return;
    }

    void client_worker::cancel()
    {
        cout << "CLIENT_WORKER CANCELED" << endl;
        if( f_receiver_state == k_running )
        {
            f_receiver->cancel();
        }
        if( f_writer_state == k_running )
        {
            f_writer->cancel();
        }
        return;
    }

}
