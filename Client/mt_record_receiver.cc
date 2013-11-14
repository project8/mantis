#include "mt_record_receiver.hh"

#include "mt_iterator.hh"
#include "mt_record_dist.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace mantis
{

    record_receiver::record_receiver( server* a_server, buffer* a_buffer, condition* a_condition ) :
            f_server( a_server ),
            f_buffer( a_buffer ),
            f_condition( a_condition )
    {
        cout << "[record_receiver] allocating buffer..." << endl;

        iterator t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            t_it->data() = new data_type[ f_buffer->record_size() ];
            t_it->set_data_size( f_buffer->record_size() );
            ++t_it;
        }
    }

    record_receiver::~record_receiver()
    {
        cout << "[record_receiver] deallocating buffer..." << endl;

        iterator t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            delete [] t_it->data();
            ++t_it;
        }
    }

    void record_receiver::execute()
    {
        record_dist* t_record_dist = new record_dist();
        cout << "[record_receiver] waiting for incomming record connection" << endl;
        // thread is blocked by the accept call in server::get_connection
        // until an incoming connection is received
        record_dist->set_connection( f_server->get_connection() );

        iterator t_it( f_buffer );
        // pass MSG_WAITALL to recv function to block thread until a record is received
        while( t_record_dist->pull_record( t_it.object(), MSG_WAITALL ) )
        {

        }




        while( true )
        {
            t_request_dist = new request_dist();
            cout << "[record_receiver] waiting for incoming connections" << endl;
            // thread is blocked by the accept call in server::get_connection 
            // until an incoming connection is received
            t_request_dist->set_connection( f_server->get_connection() );

            cout << "[record_receiver] receiving request..." << endl;

            if( ! t_request_dist->pull_request() )
            {
                cerr << "[record_receiver] unable to pull run request; sending status <error>" << endl;
                t_request_dist->get_status()->set_state( status_state_t_error );
                delete t_request_dist->get_connection();
                delete t_request_dist;
            }
            else
            {
                cout << "[record_receiver] sending status <acknowledged>..." << endl;

                t_request_dist->get_status()->set_state( status_state_t_acknowledged );
                t_request_dist->push_status();

                cout << "[record_receiver] queuing request..." << endl;

                t_request_dist->get_status()->set_state( status_state_t_waiting );
                f_request_queue->to_back( t_request_dist );


                // if the queue condition is waiting, release it
                if( f_condition->is_waiting() == true )
                {
                    cout << "[record_receiver] releasing queue condition" << endl;
                    f_condition->release();
                }
            }

            cout << "[record_receiver] finished processing request" << endl;
        }

        return;
    }

}
