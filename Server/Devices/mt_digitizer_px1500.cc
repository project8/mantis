#include "mt_digitizer_px1500.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_iterator.hh"

#include "response.pb.h"

#include <cmath> // for ceil()
#include <cstring>
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL
#include <sstream>

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_px1500" );

    MT_REGISTER_DIGITIZER( digitizer_px1500, "px1500" );

    const unsigned digitizer_px1500::s_data_type_size = sizeof( digitizer_px1500::data_type );
    unsigned digitizer_px1500::data_type_size_px1500()
    {
        return digitizer_px1500::s_data_type_size;
    }

    digitizer_px1500::digitizer_px1500() :
            //f_semaphore( NULL ),
            f_buffer( NULL ),
            f_condition( NULL ),
            f_allocated( false ),
            f_handle(),
            f_start_time( 0 ),
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_canceled( false ),
            f_cancel_condition()
    {
        get_calib_params( px1500_bits, s_data_type_size, px1500_min_val, px1500_range, &f_params );
        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_px1500", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_px1500 is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
        */
    }

    digitizer_px1500::~digitizer_px1500()
    {
        if( f_buffer != NULL ) deallocate( f_buffer );
        }
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
        */
    }

    bool digitizer_px1500::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        int t_result;

        MTINFO( mtlog, "connecting to digitizer card..." );

        t_result = ConnectToDevicePX4( &f_handle, 1 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to connect to digitizer card: " );
            return false;
        }

        //MTINFO( mtlog, "setting power up defaults..." );

        t_result = SetPowerupDefaultsPX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to enter default state: " );
            return false;
        }

        MTINFO( mtlog, "allocating dma buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                block* t_new_block = new block();
                t_result = AllocateDmaBufferPX4( f_handle, f_buffer->block_size(), t_new_block->handle() );
                if( t_result != SIG_SUCCESS )
                {
                    std::stringstream t_buff;
                    t_buff << "failed to allocate dma memory for block " << index <<": ";
                    DumpLibErrorPX4( t_result, t_buff.str().c_str() );
                    return false;
                }
                t_new_block->set_data_size( f_buffer->block_size() );
                t_new_block->set_data_nbytes( f_buffer->block_size() );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        f_allocated = true;
        return true;
    }

    bool digitizer_px1500::deallocate( buffer* a_buffer )
    {
        if( f_allocated && a_buffer == f_buffer )
        {
            int t_result;

            MTINFO( mtlog, "deallocating dma buffer..." );

            iterator t_it( f_buffer );
            for( size_t index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }
            f_allocated = false;
            f_buffer = NULL;

            MTINFO( mtlog, "disconnecting from digitizer card..." );

            t_result = DisconnectFromDevicePX4( f_handle );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to disconnect from digitizer card: " );
                return false;
            }
            return true;
        }
        MTERROR( mtlog, "Cannot deallocate buffer that was not allocated by this digitizer" );
        return false;
    }

    bool digitizer_px1500::initialize( request* a_request )
    {
        int t_result;

        //MTINFO( mtlog, "resetting counters..." );

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->block_size()) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        //MTINFO( mtlog, "setting run mode..." );

        if( a_request->mode() == request_mode_t_single )
        {
            t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_SINGLE_CH1 );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to activate channel 1: " );
                return false;
            }

        }
        if( a_request->mode() == request_mode_t_dual_separate || a_request->mode() == request_mode_t_dual_interleaved )
        {
            t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_DUAL_1_2 );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to activate channels 1 and 2: " );
                return false;
            }
        }

        //MTINFO( mtlog, "setting clock rate..." );

        t_result = SetInternalAdcClockRatePX4( f_handle, a_request->rate() );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to set clock rate: " );
            return false;
        }

        return true;
    }
    void digitizer_px1500::execute()
    {
        iterator t_it( f_buffer, "dig-px1500" );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        //MTINFO( mtlog, "waiting" );

        f_condition->wait();

        MTINFO( mtlog, "loose at <" << t_it.index() << ">" );

        //start acquisition
        if( start() == false )
        {
            MTERROR( mtlog, "unable to start acquisition" );
            return;
        }

        //start timing
        get_time_monotonic( &t_live_start_time );
        f_start_time = time_to_nsec( t_live_start_time );

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
                if( f_canceled.load() )
                {
                    MTINFO( mtlog, "was canceled mid-run" );
                    f_cancel_condition.release();
                }
                else
                {
                    MTINFO( mtlog, "finished normally" );
                }
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
                MTINFO( mtlog, "finished abnormally because acquisition failed" );
                return;
            }

            t_it->set_acquired();

            if( +t_it == false )
            {
                MTINFO( mtlog, "blocked at <" << t_it.index() << ">" );

                //stop live timer
                get_time_monotonic( &t_live_stop_time );

                //accumulate live time
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                if( stop() == false )
                {
                    //GET OUT
                    MTINFO( mtlog, "finished abnormally because halting streaming failed" );
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
                    MTINFO( mtlog, "finished abnormally because starting streaming failed" );
                    return;
                }

                //increment block
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                MTINFO( mtlog, "loose at <" << t_it.index() << ">" );
            }
        }

        return;
    }
    void digitizer_px1500::cancel()
    {
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        return;
    }
    void digitizer_px1500::finalize( response* a_response )
    {
        //MTINFO( mtlog, "calculating statistics..." );

        a_response->set_digitizer_records( f_record_count );
        a_response->set_digitizer_acquisitions( f_acquisition_count );
        a_response->set_digitizer_live_time( (double) f_live_time * SEC_PER_NSEC );
        a_response->set_digitizer_dead_time( (double) f_dead_time * SEC_PER_NSEC );
        a_response->set_digitizer_megabytes( (double) (4 * f_record_count) );
        a_response->set_digitizer_rate( a_response->digitizer_megabytes() / a_response->digitizer_live_time() );

        return;
    }

    bool digitizer_px1500::start()
    {
        int t_result = BeginBufferedPciAcquisitionPX4( f_handle, PX4_FREE_RUN );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to begin dma acquisition: " );
            return false;
        }

        return true;
    }
    bool digitizer_px1500::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );

        int t_result = GetPciAcquisitionDataFastPX4( f_handle, f_buffer->block_size(), a_block->data_bytes(), 0 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to acquire dma data over pci: " );
            t_result = EndBufferedPciAcquisitionPX4( f_handle );
            return false;
        }

        // the timestamp is acquired after the data is transferred to avoid the problem on the px1500 where
        // the first record can take unusually long to be acquired.
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) - f_start_time );

        ++f_record_count;

        return true;
    }
    bool digitizer_px1500::stop()
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

    unsigned digitizer_px1500::data_type_size()
    {
        return digitizer_px1500::s_data_type_size;
    }


    bool digitizer_px1500::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_px1500::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }


    //***********************************
    // Block Cleanup px1500
    //***********************************

    block_cleanup_px1500::block_cleanup_px1500( byte_type* a_memblock, HPX4* a_dig_ptr ) :
        f_triggered( false ),
        f_memblock( a_memblock ),
        f_dig_ptr( a_dig_ptr )
    {}
    block_cleanup_px1500::~block_cleanup_px1500()
    {}
    bool block_cleanup_px1500::delete_memblock()
    {
        if( f_triggered ) return true;
        int t_result = FreeDmaBufferPX4( *f_dig_ptr, reinterpret_cast< digitizer_px1500::data_type* >( f_memblock ) );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to deallocate dma memory: " );
            return false;
        }
        f_triggered = true;
        return true;
    }


    //***********************************
    // test_digitizer_px1500
    //***********************************

    bool test_digitizer_px1500::run_test()
    {
        int t_result;
        HPX4 f_handle;

        MTINFO( mtlog, "beginning allocation phase" );

        MTDEBUG( mtlog, "connecting to digitizer card..." );

        t_result = ConnectToDevicePX4( &f_handle, 1 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to connect to digitizer card: " );
            return false;
        }

        MTDEBUG( mtlog, "setting power up defaults..." );

        t_result = SetPowerupDefaultsPX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to enter default state: " );
            return false;
        }

        MTDEBUG( mtlog, "allocating dma buffer..." );

        block* t_block = NULL;
        // this is the minimum record size for the px1500
        unsigned t_rec_size = 16384;

        try
        {
            t_block = new block();
            t_result = AllocateDmaBufferPX4( f_handle, t_rec_size, t_block->handle() );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to allocate dma memory: " );
                return false;
            }
            t_block->set_data_size( t_rec_size );
            t_block->set_data_nbytes( t_rec_size );
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "allocation complete!\n" );



        MTINFO( mtlog, "beginning initialization phase" );

        MTDEBUG( mtlog, "setting run mode..." );

        t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_SINGLE_CH1 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to activate channel 1: " );
            return false;
        }

        MTDEBUG( mtlog, "setting clock rate..." );

        t_result = SetInternalAdcClockRatePX4( f_handle, 200. );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to set clock rate: " );
            return false;
        }

        MTINFO( mtlog, "initialization complete!\n" );


        MTINFO( mtlog, "beginning run phase" );

        MTDEBUG( mtlog, "beginning acquisition" );

        t_result = BeginBufferedPciAcquisitionPX4( f_handle, PX4_FREE_RUN );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to begin dma acquisition: " );
            return false;
        }

        MTDEBUG( mtlog, "acquiring a record" );

        t_result = GetPciAcquisitionDataFastPX4( f_handle, t_rec_size, t_block->data_bytes(), 0 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to acquire dma data over pci: " );
            t_result = EndBufferedPciAcquisitionPX4( f_handle );
            return false;
        }

        MTDEBUG( mtlog, "ending acquisition..." );

        t_result = EndBufferedPciAcquisitionPX4( f_handle );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to end dma acquisition: " );
            return false;
        }

        std::stringstream t_str_buff;
        for( unsigned i = 0; i < 99; ++i )
        {
            t_str_buff << t_block->data_bytes()[ i ] << ", ";
        }
        t_str_buff << t_block->data_bytes()[ 99 ];
        MTDEBUG( mtlog, "the first 100 samples taken:\n" << t_str_buff.str() );

        MTINFO( mtlog, "run complete!\n" );


        MTINFO( mtlog, "beginning finalization phase" );

        MTDEBUG( mtlog, "deallocating dma buffer" );

        delete t_block;

        MTINFO( mtlog, "finalization complete!\n" );


        return true;
    }

}
