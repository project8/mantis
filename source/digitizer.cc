#include "digitizer.hh"

#include "time.hh"

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

        cout << "[digitizer] setting one active channel..." << endl;

        t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_SINGLE_CH1 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to activate channel 1: " );
            exit( -1 );
        }

        cout << "[digitizer] allocating dma buffer..." << endl;

        iterator t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            t_result = AllocateDmaBufferPX4( f_handle, 4194304, t_it->data() );
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

    void digitizer::initialize( run* a_run )
    {
        int t_result;

        request& t_request = a_run->get_request();

        cout << "[digitizer] resetting counters..." << endl;

        f_record_last = (record_id_t) (ceil( (double) (t_request.rate() * t_request.duration() * 1.e3) / (double) (4194304) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        cout << "[digitizer] setting clock rate..." << endl;

        t_result = SetInternalAdcClockRatePX4( f_handle, t_request.rate() );
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

        timestamp_t t_live_start_time;
        timestamp_t t_live_stop_time;
        timestamp_t t_dead_start_time;
        timestamp_t t_dead_stop_time;

        cout << "[digitizer] waiting" << endl;

        f_condition->wait();

        cout << "[digitizer] loose at <" << t_it.index() << ">" << endl;

        //start acquisition
        if( start() == false )
        {
            return;
        }

        //start timing
        t_live_start_time = get_integral_time();

        //go go go go
        while( true )
        {
            //check if we've written enough
            if( f_record_count == f_record_last )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live microseconds
                t_live_stop_time = get_integral_time();

                f_live_time += t_live_stop_time - t_live_start_time;

                //halt the pci acquisition
                stop();

                //GET OUT
                cout << "[digitizer] finished normally" << endl;
                return;
            }

            t_it->set_acquiring();

            t_it->set_record_id( f_record_count );
            t_it->set_acquisition_id( f_acquisition_count );
            t_it->set_timestamp( get_integral_time() );

            if( acquire( t_it.object() ) == false )
            {
                //mark the block as written
                t_it->set_written();

                //get the time and update the number of live microseconds
                f_live_time += t_live_stop_time - t_live_start_time;

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

                //get the time and update the number of live microseconds
                t_live_stop_time = get_integral_time();
                f_live_time += t_live_stop_time - t_live_start_time;

                //halt the pci acquisition
                if( stop() == false )
                {
                    //GET OUT
                    cout << "[digitizer] finished abnormally because halting streaming failed" << endl;
                    return;
                }

                t_dead_start_time = get_integral_time();

                //wait
                f_condition->wait();

                //get the time and update the number of dead microseconds
                t_dead_stop_time = get_integral_time();
                f_dead_time += t_dead_stop_time - t_dead_stop_time;

                //start acquisition
                if( start() == false )
                {
                    //GET OUT
                    cout << "[digitizer] finished abnormally because starting streaming failed" << endl;
                    return;
                }

                //start timing and pop
                t_live_start_time = get_integral_time();
                ++t_it;

                cout << "[digitizer] loose at <" << t_it.index() << ">" << endl;
            }
        }

        return;
    }

    void digitizer::finalize( run* a_run )
    {
        response& t_response = a_run->get_response();

        t_response.set_digitizer_records( f_record_count );
        t_response.set_digitizer_acquisitions( f_acquisition_count );
        t_response.set_digitizer_live_time( (double) (f_live_time) / (double) (1000000) );
        t_response.set_digitizer_dead_time( (double) (f_dead_time) / (double) (1000000) );
        t_response.set_digitizer_megabytes( (double) (4 * f_record_count) );
        t_response.set_digitizer_rate( (double) (4000000 * f_record_count) / (double) (f_live_time) );

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
    bool digitizer::acquire( block* a_block )
    {
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
