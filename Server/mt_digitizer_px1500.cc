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

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_px1500" );

    static registrar< digitizer, digitizer_px1500 > s_px1500_registrar( "px1500" );
    static registrar< test_digitizer, test_digitizer_px1500 > s_test_px1500_registrar( "px1500" );

    const unsigned digitizer_px1500::s_bit_depth = 8;
    unsigned digitizer_px1500::bit_depth_px1500()
    {
        return digitizer_px1500::s_bit_depth;
    }

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
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_canceled( false ),
            f_cancel_condition()
    {
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
        if( f_allocated )
        {
            int t_result;

            MTINFO( mtlog, "deallocating dma buffer..." );

            iterator t_it( f_buffer );
            for( size_t index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }

            MTINFO( mtlog, "disconnecting from digitizer card..." );

            t_result = DisconnectFromDevicePX4( f_handle );
            if( t_result != SIG_SUCCESS )
            {
                DumpLibErrorPX4( t_result, "failed to disconnect from digitizer card: " );
                exit( -1 );
            }
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
                typed_block< data_type >* t_new_block = new typed_block< data_type >();
                t_result = AllocateDmaBufferPX4( f_handle, f_buffer->record_size(), t_new_block->handle() );
                if( t_result != SIG_SUCCESS )
                {
                    DumpLibErrorPX4( t_result, "failed to allocate dma memory: " );
                    return false;
                }
                t_new_block->set_data_size( f_buffer->record_size() );
                t_new_block->set_cleanup( new block_cleanup_px1500( t_new_block->data(), &f_handle ) );
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
    bool digitizer_px1500::initialize( request* a_request )
    {
        int t_result;

        //MTINFO( mtlog, "resetting counters..." );

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->record_size()) ));
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
        typed_iterator< data_type > t_it( f_buffer );

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

            if( acquire( t_it.typed_object(), t_stamp_time ) == false )
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
    bool digitizer_px1500::acquire( typed_block< data_type >* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );

        int t_result = GetPciAcquisitionDataFastPX4( f_handle, f_buffer->record_size(), a_block->data(), 0 );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to acquire dma data over pci: " );
            t_result = EndBufferedPciAcquisitionPX4( f_handle );
            return false;
        }

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

    bool digitizer_px1500::write_mode_check( request_file_write_mode_t mode )
    {
        return true;
    }

    unsigned digitizer_px1500::bit_depth()
    {
        return digitizer_px1500::s_bit_depth;
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

    block_cleanup_px1500::block_cleanup_px1500( digitizer_px1500::data_type* a_data, HPX4* a_dig_ptr ) :
        f_triggered( false ),
        f_data( a_data ),
        f_dig_ptr( a_dig_ptr )
    {}
    block_cleanup_px1500::~block_cleanup_px1500()
    {}
    bool block_cleanup_px1500::delete_data()
    {
        if( f_triggered ) return true;
        int t_result = FreeDmaBufferPX4( *f_dig_ptr, f_data );
        if( t_result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( t_result, "failed to deallocate dma memory: " );
            return false;
        }
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

        typed_block< digitizer_px1500::data_type >* t_block = NULL;
        // this is the minimum record size for the px1500
        unsigned t_rec_size = 16384;

        try
        {
                t_block = new typed_block< digitizer_px1500::data_type >();
                t_result = AllocateDmaBufferPX4( f_handle, t_rec_size, t_block->handle() );
                if( t_result != SIG_SUCCESS )
                {
                    DumpLibErrorPX4( t_result, "failed to allocate dma memory: " );
                    return false;
                }
                t_block->set_data_size( t_rec_size );
                t_block->set_cleanup( new block_cleanup_px1500( t_block->data(), &f_handle ) );
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

        t_result = GetPciAcquisitionDataFastPX4( f_handle, t_rec_size, t_block->data(), 0 );
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
	    t_str_buff << t_block->data()[ i ] << ", ";
	  }
	t_str_buff << t_block->data()[ 99 ];
        MTDEBUG( mtlog, "the first 100 samples taken:\n" << t_str_buff.str() );

        MTINFO( mtlog, "run complete!\n" );


        MTINFO( mtlog, "beginning finalization phase" );

        MTDEBUG( mtlog, "deallocating dma buffer" );

        delete t_block;

        MTINFO( mtlog, "finalization complete!\n" );


        return true;
    }

}
