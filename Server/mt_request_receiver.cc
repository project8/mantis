#include "mt_request_receiver.hh"

#include "mt_buffer.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace mantis
{

    request_receiver::request_receiver( server* a_server, request_queue* a_request_queue, condition* a_condition ) :
            f_server( a_server ),
            f_request_queue( a_request_queue ),
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
        request_dist* t_request_dist;

        while( true )
        {
            t_request_dist = new request_dist();
            cout << "[request_receiver] waiting for incoming connections" << endl;
            // thread is blocked by the accept call in server::get_connection 
            // until an incoming connection is received
            t_request_dist->set_connection( f_server->get_connection() );

            cout << "[request_receiver] receiving request..." << endl;

            // use blocking option for pull request
            if( ! t_request_dist->pull_request( MSG_WAITALL ) )
            {
                cerr << "[request_receiver] unable to pull run request; sending server status <error>" << endl;
                t_request_dist->get_status()->set_state( status_state_t_error );
                t_request_dist->push_status();
                delete t_request_dist->get_connection();
                delete t_request_dist;
            }
            else
            {
                cout << "[request_receiver] sending server status <acknowledged>..." << endl;

                t_request_dist->get_status()->set_state( status_state_t_acknowledged );
                t_request_dist->get_status()->set_buffer_size( f_buffer_size );
                t_request_dist->get_status()->set_record_size( f_record_size );
                t_request_dist->get_status()->set_data_chunk_size( f_data_chunk_size );
                t_request_dist->push_status();

                // TODO: wait here until client sends client_status ready

                cout << "[request_receiver] queuing request..." << endl;

                t_request_dist->get_status()->set_state( status_state_t_waiting );
                f_request_queue->to_back( t_request_dist );


                // if the queue condition is waiting, release it
                if( f_condition->is_waiting() == true )
                {
                    cout << "[request_receiver] releasing queue condition" << endl;
                    f_condition->release();
                }
            }

            cout << "[request_receiver] finished processing request" << endl;
        }

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
