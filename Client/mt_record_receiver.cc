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
            *( t_it->handle() ) = new data_type[ f_buffer->record_size() ];
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
        t_record_dist->set_connection( f_server->get_connection() );

        iterator t_it( f_buffer );
        // pass MSG_WAITALL to recv function to block thread until a record is received
        while( t_record_dist->pull_record( t_it.object(), MSG_WAITALL ) )
        {
            cout << "[record_receiver] record received" << endl;

            //TODO: loop exit condition
            //TODO: livetime/deadtime calculation, as in digitizer_px1500
        }

        cout << "[record_receiver] finished processing records" << endl;

        delete t_record_dist->get_connection();
        delete t_record_dist;

        return;
    }

}
