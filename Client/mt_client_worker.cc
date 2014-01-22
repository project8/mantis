#include "mt_client_worker.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_thread.hh"

namespace mantis
{
    MTLOGGER( mtlog, "client_worker" );

    client_worker::client_worker( request* a_request, record_receiver* a_receiver, writer* a_writer, condition* a_buffer_condition ) :
                    f_receiver( a_receiver ),
                    f_writer( a_writer ),
                    f_buffer_condition( a_buffer_condition ),
                    f_is_done( false ),
                    f_receiver_state( k_inactive ),
                    f_writer_state( k_running )
    {
        if(! f_writer->initialize( a_request ) )
        {
            throw exception() << "writer could not be initialized\n";
        }
    }

    client_worker::~client_worker()
    {
    }

    void client_worker::execute()
    {
        f_is_done.store( false );

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

        MTINFO( mtlog, "running..." );

        t_receiver_thread->join();
        f_receiver_state = k_inactive;
        t_writer_thread->join();
        f_writer_state = k_inactive;

        delete t_receiver_thread;
        delete t_writer_thread;

        MTINFO( mtlog, "finalizing..." );

        response t_response;
        f_receiver->finalize( &t_response );
        f_writer->finalize( &t_response );
        t_response.set_state( response_state_t_complete );
        MTINFO( mtlog, "record_receiver summary:\n"
            << "  record count: " << t_response.digitizer_records() << " [#]\n"
            << "  acquisition count: " << t_response.digitizer_acquisitions() << " [#]\n"
            << "  live time: " << t_response.digitizer_live_time() << " [sec]\n"
            << "  dead time: " << t_response.digitizer_dead_time() << " [sec]\n"
            << "  megabytes: " << t_response.digitizer_megabytes() << " [Mb]\n"
            << "  rate: " << t_response.digitizer_rate() << " [Mb/sec]\n" );

        MTINFO( mtlog, "file_writer summary:\n"
            << "  record count: " << t_response.writer_records() << " [#]\n"
            << "  acquisition count: " << t_response.writer_acquisitions() << " [#]\n"
             << "  live time: " << t_response.writer_live_time() << " [sec]\n"
            << "  megabytes: " << t_response.writer_megabytes() << "[Mb]\n"
            << "  rate: " << t_response.writer_rate() << " [Mb/sec]\n"  );

        f_is_done.store( true );

        return;
    }

    void client_worker::cancel()
    {
        //cout << "CLIENT_WORKER CANCELED" );
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

    bool client_worker::is_done()
    {
        return f_is_done.load();
    }

}
