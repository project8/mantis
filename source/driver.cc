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
            sleep( 1 );

            if( f_queue->is_empty() == true )
            {
                cout << "[driver] waiting..." << endl;
            }
            else
            {
                cout << "[driver] spooling..." << endl;
                t_active = f_queue->from_back();
                t_active->get_response().set_status( response_status_t_ready );
                t_active->push_response();

                t_active->get_response().set_status( response_status_t_done );
            }


            f_queue->push_response();
        }
    }

}
