#include "mt_request_receiver.hh"

#include "mt_buffer.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace mantis
{

    request_receiver::request_receiver( server* a_server, run_queue* a_run_queue, condition* a_condition ) :
            f_server( a_server ),
            f_run_queue( a_run_queue ),
            f_condition( a_condition ),
            f_buffer_size( 512 ),
            f_record_size( 419304 ),
            f_data_chunk_size( 1024 )
    {
    }

    request_receiver::~request_receiver()
    {
    }

    void request_receiver::execute()
    {
        run_context_dist* t_run_context;

        while( true )
        {
            t_run_context = new run_context_dist();
            cout << "[request_receiver] waiting for incoming connections" << endl;
            // thread is blocked by the accept call in server::get_connection 
            // until an incoming connection is received
            t_run_context->set_connection( f_server->get_connection() );

            cout << "[request_receiver] receiving request..." << endl;

            // use blocking option for pull request
            if( ! t_run_context->pull_request( MSG_WAITALL ) )
            {
                cerr << "[request_receiver] unable to pull run request; sending server status <error>" << endl;
                t_run_context->get_status()->set_state( status_state_t_error );
                t_run_context->push_status();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            cout << "[request_receiver] sending server status <acknowledged>..." << endl;

            t_run_context->get_status()->set_state( status_state_t_acknowledged );
            t_run_context->get_status()->set_buffer_size( f_buffer_size );
            t_run_context->get_status()->set_record_size( f_record_size );
            t_run_context->get_status()->set_data_chunk_size( f_data_chunk_size );
            t_run_context->push_status();

            cout << "[request_receiver] waiting for client readiness..." << endl;

            if( ! t_run_context->pull_client_status( MSG_WAITALL ) )
            {
                cerr << "[request_receiver] unable to pull client status; sending server status <error>" << endl;
                t_run_context->get_status()->set_state( status_state_t_error );
                t_run_context->push_status();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }
            if( ! t_run_context->get_client_status()->state() == client_status_state_t_ready )
            {
                cerr << "[request_receiver] client did not get ready; sending server status <error>" << endl;
                t_run_context->get_status()->set_state( status_state_t_error );
                t_run_context->push_status();
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            cout << "[request_receiver] queuing request..." << endl;

            t_run_context->get_status()->set_state( status_state_t_waiting );
            f_run_queue->to_back( t_run_context );


            // if the queue condition is waiting, release it
            if( f_condition->is_waiting() == true )
            {
                cout << "[request_receiver] releasing queue condition" << endl;
                f_condition->release();
            }

            cout << "[request_receiver] finished processing request" << endl;
        }

        return;
    }

    void request_receiver::cancel()
    {
        return;
    }

    size_t request_receiver::get_buffer_size() const
    {
        return f_buffer_size;
    }
    void request_receiver::set_buffer_size( size_t size )
    {
        f_buffer_size = size;
        return;
    }

    size_t request_receiver::get_record_size() const
    {
        return f_record_size;
    }
    void request_receiver::set_record_size( size_t size )
    {
        f_record_size = size;
        return;
    }

    size_t request_receiver::get_data_chunk_size() const
    {
        return f_data_chunk_size;
    }
    void request_receiver::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }

}
