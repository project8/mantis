#include "receiver.hh"
#include "server.hh"
#include "queue.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    receiver::receiver( server* a_server, queue* a_queue ) :
            f_server( a_server ),
            f_queue( a_queue )
    {
    }

    receiver::~receiver()
    {
    }

    void receiver::execute()
    {
        connection* t_connection;
        run* t_run;

        while( true )
        {
            usleep( 100000 );

            t_run = new run( f_server->get_connection() );

            cout << "[receiver] got a connection..." << endl;

            t_run->pull_request();

            request& t_request = t_run->get_request();
            response& t_response = t_run->get_response();
            t_response.set_rate( t_request.rate() );
            t_response.set_duration( t_request.duration() );
            t_response.set_file( t_request.file() );
            t_response.set_date( t_request.date() );
            t_response.set_description( t_request.description() );
            t_response.set_status( response_status_t_acknowledged );

            t_run->push_response();

            f_queue->to_back( t_run );
        }

        return;
    }

}
