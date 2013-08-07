#include "driver.hh"
#include "queue.hh"

#include <cstddef>

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    driver::driver( queue* a_queue, digitizer* a_digitizer, writer* a_writer ) :
            f_queue( a_queue ),
            f_digitizer( a_digitizer ),
            f_writer( a_writer )
    {
    }

    driver::~driver()
    {
    }

    void driver::execute()
    {
        run* t_active = NULL;

        while( true )
        {
            usleep( 10000 );

            if( t_active != NULL )
            {
                if( t_active->get_response().status() == response_status_t_running )
                {
                    t_active->push_response();
                    continue;
                }

                if( t_active->get_response().status() == response_status_t_done )
                {
                    t_active->push_response();
                    delete t_active;
                    t_active = NULL;
                    continue;
                }
            }

            if( f_queue->is_empty() == true )
            {
                cout << "[driver] waiting..." << endl;
            }
            else
            {
                cout << "[driver] spooling..." << endl;
                t_active = f_queue->from_front();
                t_active->get_response().set_status( response_status_t_ready );
                t_active->push_response();
                f_queue->for_each( &run::push_response );
                t_active->get_response().set_status( response_status_t_done );
            }
        }
    }

}
