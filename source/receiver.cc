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
            t_context = new context( f_server->get_connection() );

            cout << "[receiver] got a connection..." << endl;

            t_context->pull_request();
            t_context->get_status()->set_state( status_state_t_acknowledged );
            t_context->push_status();

            f_queue->to_back( t_context );

            if( f_condition->is_waiting() == true )
            {
                f_condition->release();
            }
        }

        return;
    }

}
