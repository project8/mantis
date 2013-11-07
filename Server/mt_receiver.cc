#include "mt_receiver.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace mantis
{

    receiver::receiver( server* a_server, run_queue* a_run_queue, condition* a_condition ) :
            f_server( a_server ),
            f_run_queue( a_run_queue ),
            f_condition( a_condition )
    {
    }

    receiver::~receiver()
    {
    }

    void receiver::execute()
    {
        run_context* t_run_context;

        while( true )
        {
            t_run_context = new run_context();
            t_run_context->set_connection( f_server->get_connection() );

            cout << "[receiver] receiving request..." << endl;

            if( ! t_run_context->pull_request() )
            {
                cerr << "[receiver] unable to pull run request" << endl;
                cout << "[receiver] sending status <error>" << endl;
                t_run_context->get_status()->set_state( status_state_t_error );
                t_run_context->push_status();
                delete t_run_context;
                continue;
            }

            cout << "[receiver] sending status <acknowledged>..." << endl;

            t_run_context->get_status()->set_state( status_state_t_acknowledged );
            t_run_context->push_status();

            cout << "[receiver] queuing request..." << endl;

            t_run_context->get_status()->set_state( status_state_t_waiting );
            f_run_queue->to_back( t_run_context );

            if( f_condition->is_waiting() == true )
            {
                f_condition->release();
            }
        }

        return;
    }

}
