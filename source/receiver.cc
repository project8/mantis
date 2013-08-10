#include "receiver.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    receiver::receiver( server* a_server, queue* a_queue, condition* a_condition ) :
            f_server( a_server ),
            f_queue( a_queue ),
            f_condition( a_condition )
    {
    }

    receiver::~receiver()
    {
    }

    void receiver::execute()
    {
        context* t_context;

        while( true )
        {
            t_context = new context();
            t_context->set_connection( f_server->get_connection() );

            cout << "[receiver] got a connection..." << endl;


            cout << "[receiver] pulling request..." << endl;
            t_context->pull_request();

            cout << "[receiver] requested clock rate was <" << t_context->get_request()->rate() << ">" << endl;

            t_context->get_status()->set_state( status_state_t_acknowledged );

            cout << "[receiver] pushing status..." << endl;
            t_context->push_status();

            t_context->get_status()->set_state( status_state_t_waiting );
            f_queue->to_back( t_context );

            if( f_condition->is_waiting() == true )
            {
                f_condition->release();
            }
        }

        return;
    }

}
