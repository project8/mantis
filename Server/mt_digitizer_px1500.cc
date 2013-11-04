#include "mt_digitizer_px1500.hh"

#include "mt_iterator.hh"

#include <cstdlib> // for exit()
#include <cmath> // for ceil()
#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    digitizer::digitizer( buffer* a_buffer, condition* a_condition ) :
            f_buffer( a_buffer ),
            f_condition( a_condition ),
            f_handle(),
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 )
    {
        int t_result;

        cout << "[digitzer] connecting to digitizer card..." << endl;

        t_result = ConnectToDevicePX4( &f_handle, 1 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to connect to digitizer card: " );
            exit( -1 );
        }

        cout << "[digitizer] setting power up defaults..." << endl;

        t_result = SetPowerupDefaultsPX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to enter default state: " );
            exit( -1 );
        }

        cout << "[digitizer] allocating dma buffer..." << endl;

        iterator t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            t_result = AllocateDmaBufferPX4( f_handle, 4194304, t_it->handle() );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to allocate dma memory: " );
                exit( -1 );
            }
            ++t_it;
        }
    }
    digitizer::~digitizer()
    {
        int t_result;

        cout << "[digitizer] deallocating dma buffer..." << endl;

        iterator t_it( f_buffer );
        for( size_t Index = 0; Index < f_buffer->size(); Index++ )
        {
            t_result = FreeDmaBufferPX4( f_handle, t_it->data() );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to deallocate dma memory: " );
                exit( -1 );
            }
            ++t_it;
        }

        cout << "[digitizer] disconnecting from digitizer card..." << endl;

        t_result = DisconnectFromDevicePX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to disconnect from digitizer card: " );
            exit( -1 );
        }
    }

    void digitizer::initialize( request* a_request )
    {
        int t_result;

        cout << "[digitizer] resetting counters..." << endl;

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (4194304) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        cout << "[digitizer] setting run mode..." << endl;

        if( a_request->mode() == request_mode_t_single )
        {
            t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_SINGLE_CH1 );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to activate channel 1: " );
                exit( -1 );
            }

        }
        if( a_request->mode() == request_mode_t_dual_separate || a_request->mode() == request_mode_t_dual_interleaved )
        {
            t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_DUAL_1_2 );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to activate channels 1 and 2: " );
                exit( -1 );
            }
        }

        cout << "[digitizer] setting clock rate..." << endl;

        t_result = SetInternalAdcClockRatePX4( f_handle, a_request->rate() );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to set clock rate: " );
            exit( -1 );
        }

        return;
    }
    void digitizer::execute()
    {
        iterator t_it( f_buffer );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        cout << "[digitizer] waiting" << endl;

        f_condition->wait();

        cout << "[digitizer] loose at <" << t_it.index() << ">" << endl;

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
            if( f_record_count == f_record_last )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live nanoseconds
                get_time_monotonic( &t_live_stop_time );		

                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                stop();

                //GET OUT
                cout << "[digitizer] finished normally" << endl;
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
                cout << "[digitizer] finished abnormally because acquisition failed" << endl;
                return;
            }

            t_it->set_acquired();

            if( +t_it == false )
            {
                cout << "[digitizer] blocked at <" << t_it.index() << ">" << endl;

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
                    cout << "[digitizer] finished abnormally because starting streaming failed" << endl;
                    return;
                }

                //increment block
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                cout << "[digitizer] loose at <" << t_it.index() << ">" << endl;
            }
        }

        return;
    }
    void digitizer::finalize( response* a_response )
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

    bool digitizer::start()
    {
        int t_result = BeginBufferedPciAcquisitionPX4( f_handle, PX4_FREE_RUN );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to begin dma acquisition: " );
            return false;
        }

        return true;
    }
    bool digitizer::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );

        int t_result = GetPciAcquisitionDataFastPX4( f_handle, 4194304, a_block->data(), 0 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to acquire dma data over pci: " );
            t_result = EndBufferedPciAcquisitionPX4( f_handle );
            return false;
        }

        ++f_record_count;

        return true;
    }
    bool digitizer::stop()
    {
        int t_result = EndBufferedPciAcquisitionPX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to end dma acquisition: " );
            return false;
        }
        ++f_acquisition_count;
        return true;
    }

}
