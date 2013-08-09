#include "worker.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    worker::worker( run* a_run, digitizer* a_digitizer, writer* a_writer ) :
            f_run( a_run ),
            f_digitizer( a_digitizer ),
            f_writer( a_writer )
    {
    }

    worker::~worker()
    {
    }

    void worker::execute()
    {
        cout << "[worker] initializing run..." << endl;

        request& t_request = f_run->get_request();
        response& t_response = f_run->get_response();



        cout << "[worker] initializing digitizer..." << endl;

        f_digitizer->initialize( f_run );

        cout << "[worker] initializing writer..." << endl;

        f_writer->initialize( f_run );

        cout << "[mantis standalone] starting threads..." << endl;

        thread* t_digitizer_thread = new thread( t_digitizer );
        thread* t_writer_thread = new thread( t_writer );

        t_digitizer_thread->start();
        t_writer_thread->start();

        t_digitizer_thread->join();
        t_writer_thread->join();

        cout << "[mantis standalone] cleaning threads..." << endl;

        delete t_digitizer_thread;
        delete t_writer_thread;

        cout << "[mantis standalone] finalizing digitizer..." << endl;

        t_digitizer->finalize( t_run );

        cout << "[mantis standalone] finalizing writer..." << endl;

        t_writer->finalize( t_run );
    }

}
