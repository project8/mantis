#include "mt_record_receiver.hh"

#include "mt_iterator.hh"
#include "mt_record_dist.hh"
#include "response.pb.h"

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
            f_condition( a_condition ),
            f_record_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_data_chunk_size( 1024 )
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
        t_record_dist->set_data_chunk_size( f_data_chunk_size );
        cout << "[record_receiver] waiting for incoming record connection" << endl;
        // thread is blocked by the accept call in server::get_connection
        // until an incoming connection is received
        t_record_dist->set_connection( f_server->get_connection() );

        iterator t_it( f_buffer );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        f_record_count = 0;

        cout << "[record_receiver] waiting" << endl;

        f_condition->wait();

        cout << "[record_receiver] loose at <" << t_it.index() << ">" << endl;

        //start timing
        get_time_monotonic( &t_live_start_time );

        //go go go go
        while( true )
        {
            t_it->set_acquiring();

            if( receive( t_it.object(), t_record_dist ) == false )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live microseconds
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //GET OUT
                cout << "[record_receiver] finished abnormally because receive failed" << endl;
                return;
            }

            ++f_record_count;

            //a zero-length data array indicates the end of the data
            if( t_it.object()->get_data_size() == 0 )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live nanoseconds
                get_time_monotonic( &t_live_stop_time );

                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //GET OUT
                cout << "[record_receiver] finished normally" << endl;
                break;
            }

            t_it->set_acquired();

            if( +t_it == false )
            {
                cout << "[record_receiver] blocked at <" << t_it.index() << ">" << endl;

                //stop live timer
                get_time_monotonic( &t_live_stop_time );

                //accumulate live time
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //start dead timer
                get_time_monotonic( &t_dead_start_time );

                //wait
                f_condition->wait();

                //stop dead timer
                get_time_monotonic( &t_dead_stop_time );

                //accumulate dead time
                f_dead_time += time_to_nsec( t_dead_stop_time ) - time_to_nsec( t_dead_start_time );

                //increment block
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                cout << "[digitizer] loose at <" << t_it.index() << ">" << endl;
            }
        }

        cout << "[record_receiver] finished processing records" << endl;

        delete t_record_dist->get_connection();
        delete t_record_dist;

        return;
    }

    void record_receiver::cancel()
    {
        return;
    }

    void record_receiver::finalize( response* a_response )
    {
        cout << "[digitizer] calculating statistics..." << endl;

        a_response->set_digitizer_records( f_record_count );
        a_response->set_digitizer_live_time( (double) f_live_time * SEC_PER_NSEC );
        a_response->set_digitizer_dead_time( (double) f_dead_time * SEC_PER_NSEC );
        a_response->set_digitizer_megabytes( (double) (4 * f_record_count) );
        a_response->set_digitizer_rate( a_response->digitizer_megabytes() / a_response->digitizer_live_time() );

        return;
    }

    size_t record_receiver::get_data_chunk_size()
    {
        return f_data_chunk_size;
    }
    void record_receiver::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }

    bool record_receiver::receive( block* a_block, record_dist* a_dist )
    {
        return a_dist->pull_record( a_block, MSG_WAITALL );
    }


}
