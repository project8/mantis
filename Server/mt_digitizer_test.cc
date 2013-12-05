#include "mt_digitizer_test.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"

#include "response.pb.h"

#include <cstdlib> // for exit()
#include <cmath> // for ceil()
#include <iostream>
using std::cout;
using std::endl;
#include <string.h> // for memset()

namespace mantis
{
    static registrar< digitizer, digitizer_test > s_digtest_registrar("test");

    digitizer_test::digitizer_test() :
            f_buffer( NULL ),
            f_condition( NULL ),
            f_allocated( false ),
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_canceled( false )
    {
    }

    void digitizer_test::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        cout << "[digitizer_test] allocating buffer..." << endl;

        iterator t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            *( t_it->handle() ) = new data_type[ f_buffer->record_size() ];
            t_it->set_data_size( f_buffer->record_size() );
            ++t_it;
        }

        f_allocated = true;
        return;
    }
    digitizer_test::~digitizer_test()
    {
        if( f_allocated )
        {
            cout << "[digitzer_test] deallocating buffer..." << endl;

            iterator t_it( f_buffer );
            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                delete [] t_it->data();
                ++t_it;
            }
        }
    }

    void digitizer_test::initialize( request* a_request )
    {
        int t_result;

        cout << "[digitizer_test] resetting counters..." << endl;

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->record_size()) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        return;
    }
    void digitizer_test::execute()
    {
        iterator t_it( f_buffer );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        cout << "[digitizer_test] waiting" << endl;

        f_condition->wait();

        cout << "[digitizer_test] loose at <" << t_it.index() << ">" << endl;

        int t_old_cancel_state;
        pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

        //start acquisition
        if( start() == false )
        {
            return;
        }

        //start timing
        get_time_monotonic( &t_live_start_time );

        //go go go go
        while( true )
        {
            //check if we've written enough
            if( f_record_count == f_record_last || f_canceled.load() )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live nanoseconds
                get_time_monotonic( &t_live_stop_time );		

                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                stop();

                //GET OUT
                cout << "[digitizer_test] finished normally" << endl;
                return;
            }

            t_it->set_acquiring();

            if( acquire( t_it.object(), t_stamp_time ) == false )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live microseconds
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                stop();

                //GET OUT
                cout << "[digitizer_test] finished abnormally because acquisition failed" << endl;
                return;
            }

            t_it->set_acquired();

            if( +t_it == false )
            {
                cout << "[digitizer_test] blocked at <" << t_it.index() << ">" << endl;

                //stop live timer
                get_time_monotonic( &t_live_stop_time );

                //accumulate live time
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                if( stop() == false )
                {
                    //GET OUT
                    cout << "[digitizer] finished abnormally because halting streaming failed" << endl;
                    return;
                }

                //start dead timer
                get_time_monotonic( &t_dead_start_time );

                //wait
                f_condition->wait();

                //stop dead timer
                get_time_monotonic( &t_dead_stop_time );

                //accumulate dead time
                f_dead_time += time_to_nsec( t_dead_stop_time ) - time_to_nsec( t_dead_start_time );

                //start acquisition
                if( start() == false )
                {
                    //GET OUT
                    cout << "[digitizer_test] finished abnormally because starting streaming failed" << endl;
                    return;
                }

                //increment block
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                cout << "[digitizer_test] loose at <" << t_it.index() << ">" << endl;
            }
        }

        return;
    }
    void digitizer_test::cancel()
    {
        cout << "CANCELLING DIGITIZER TEST" << endl;
        set_canceled( true );
        return;
    }
    void digitizer_test::finalize( response* a_response )
    {
        cout << "[digitizer] calculating statistics..." << endl;

        a_response->set_digitizer_records( f_record_count );
        a_response->set_digitizer_acquisitions( f_acquisition_count );
        a_response->set_digitizer_live_time( (double) f_live_time * SEC_PER_NSEC );
        a_response->set_digitizer_dead_time( (double) f_dead_time * SEC_PER_NSEC );
        a_response->set_digitizer_megabytes( (double) (4 * f_record_count) );
        a_response->set_digitizer_rate( a_response->digitizer_megabytes() / a_response->digitizer_live_time() );

        return;
    }

    bool digitizer_test::start()
    {
        return true;
    }
    bool digitizer_test::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );

        memset( a_block->data(), f_record_count % 256, f_buffer->record_size() );

        ++f_record_count;

        return true;
    }
    bool digitizer_test::stop()
    {
        ++f_acquisition_count;
        return true;
    }

    bool digitizer_test::write_mode_check( request_file_write_mode_t mode )
    {
        return true;
    }

    bool digitizer_test::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_test::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }
}