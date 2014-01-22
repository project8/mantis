#include "mt_record_receiver.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"
#include "mt_record_dist.hh"
#include "mt_server.hh"
#include "response.pb.h"

#include <cstddef>

namespace mantis
{
    MTLOGGER( mtlog, "record_receiver" );

    record_receiver::record_receiver( server* a_server, buffer* a_buffer, condition* a_condition ) :
            f_server( a_server ),
            f_buffer( a_buffer ),
            f_condition( a_condition ),
            f_record_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_data_chunk_size( 1024 ),
            f_canceled( false )
    {
        MTINFO( mtlog, "allocating buffer..." );

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
        MTINFO( mtlog, "deallocating buffer..." );

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
        MTINFO( mtlog, "waiting for incoming record connection" );
        // thread is blocked by the accept call in server::get_connection
        // until an incoming connection is received
        t_record_dist->set_connection( f_server->get_connection() );

        iterator t_it( f_buffer );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        //timespec t_stamp_time;

        f_record_count = 0;

        MTINFO( mtlog, "waiting" );

        f_condition->wait();

        MTINFO( mtlog, "loose at <" << t_it.index() << ">" );

        //start timing
        get_time_monotonic( &t_live_start_time );

        int t_old_cancel_state;
        pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

        //go go go go
        while( true )
        {
            t_it->set_acquiring();

            if( receive( t_it.object(), t_record_dist ) == false )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live nanoseconds
                get_time_monotonic( &t_live_stop_time );

                //get the time and update the number of live microseconds
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //GET OUT
                MTINFO( mtlog, "finished abnormally because receive failed" );
                return;
            }

            ++f_record_count;

            //a zero-length data array indicates the end of the data
            if( t_it.object()->get_data_size() == 0 || f_canceled.load() )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live nanoseconds
                get_time_monotonic( &t_live_stop_time );

                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //GET OUT
                MTINFO( mtlog, "finished normally" );
                break;
            }

            t_it->set_acquired();

            if( +t_it == false )
            {
                MTINFO( mtlog, "blocked at <" << t_it.index() << ">" );

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

                MTINFO( mtlog, "loose at <" << t_it.index() << ">" );
            }

            //MTINFO( mtlog, "records received: " << f_record_count );
        }

        //MTINFO( mtlog, "finished processing records" );

        delete t_record_dist->get_connection();
        delete t_record_dist;

        return;
    }

    void record_receiver::cancel()
    {
        //cout << "RECORD_RECEIVER CANCELED" );
        f_canceled.store( true );
        return;
    }

    void record_receiver::finalize( response* a_response )
    {
        MTINFO( mtlog, "calculating statistics..." );

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
