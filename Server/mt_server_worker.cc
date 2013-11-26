#include "mt_server_worker.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_request_queue.hh"
#include "mt_thread.hh"
#include "mt_writer.hh"

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

namespace mantis
{

    server_worker::server_worker( configurator* a_config, digitizer* a_digitizer, buffer* a_buffer, request_queue* a_request_queue, condition* a_queue_condition, condition* a_buffer_condition ) :
            f_config( a_config ),
            f_digitizer( a_digitizer ),
            f_writer( NULL ),
            f_buffer( a_buffer ),
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
        request_dist* t_run_context;

        while( true )
        {
            if( f_request_queue->empty() == true )
            {
                f_queue_condition->wait();
            }

            cout << "[server_worker] sending server status <started>..." << endl;

            t_run_context = f_request_queue->from_front();
            t_run_context->get_status()->set_state( status_state_t_started );
            t_run_context->push_status();

            cout << "[server_worker] initializing..." << endl;

            f_digitizer->initialize( t_run_context->get_request() );

            cout << "[server_worker] creating writer..." << endl;

            if(! f_digitizer->write_mode_check( t_run_context->get_request()->file_write_mode() ) )
            {
                cerr << "[server_worker] unable to operate in write mode " << t_run_context->get_request()->file_write_mode() << endl;
                cerr << "                run request ignored" << endl;
                t_run_context->get_status()->set_state( status_state_t_error );
                t_run_context->push_status();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            factory< writer >* t_writer_factory = factory< writer >::get_instance();
            if( t_run_context->get_request()->file_write_mode() == request_file_write_mode_t_local )
            {
                f_writer = t_writer_factory->create( "file" );
            }
            else
            {
                f_writer = t_writer_factory->create( "network" );
            }
            f_writer->set_buffer( f_buffer, f_buffer_condition );
            f_writer->configure( f_config );
            f_writer->initialize( t_run_context->get_request() );

            cout << "[server_worker] running..." << endl;

            thread* t_digitizer_thread = new thread( f_digitizer );
            thread* t_writer_thread = new thread( f_writer );

            t_digitizer_thread->start();

            while( f_buffer_condition->is_waiting() == false )
            {
                usleep( 1000 );
            }

            t_writer_thread->start();

            t_run_context->get_status()->set_state( status_state_t_running );
            f_request_queue->to_front( t_run_context );
            t_run_context = NULL;

            t_digitizer_thread->join();
            t_writer_thread->join();

            delete t_digitizer_thread;
            delete t_writer_thread;

            cout << "[server_worker] sending server status <stopped>..." << endl;

            t_run_context = f_request_queue->from_front();
            t_run_context->get_status()->set_state( status_state_t_stopped );
            t_run_context->push_status();

            cout << "[server_worker] finalizing..." << endl;

            f_digitizer->finalize( t_run_context->get_response() );
            f_writer->finalize( t_run_context->get_response() );
            t_run_context->push_response();

            delete t_run_context->get_connection();
            delete t_run_context;
        }

        return;
    }

    void server_worker::set_writer( writer* a_writer )
    {
        f_writer = a_writer;
    }

}
