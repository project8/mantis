#include "mt_digitizer_u1084a.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_iterator.hh"

#include "response.pb.h"

#include "thorax_digital.h"

#include <cmath> // for ceil()
#include <cstring>
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL
#include <sstream>

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_u1084a" );

    MT_REGISTER_DIGITIZER( digitizer_u1084a, "u1084a" );
    MT_REGISTER_TEST_DIGITIZER( test_digitizer_u1084a, "u1084a" );

    const unsigned digitizer_u1084a::s_data_type_size = sizeof( digitizer_u1084a::data_type );
    unsigned digitizer_u1084a::data_type_size_u1084a()
    {
        return digitizer_u1084a::s_data_type_size;
    }

    void PrintU1084AError( ViSession a_handle, ViStatus a_status, const std::string& a_prepend_msg )
    {
        static char t_buff[512];
        Acqrs_errorMessage( a_handle, a_status, t_buff, 512 );
        MTERROR( mtlog, a_prepend_msg << t_buff );
        return;
    }

    digitizer_u1084a::digitizer_u1084a() :
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
        get_calib_params( u1084a_bits, s_data_type_size, u1084a_min_val, u1084a_range, &f_params );
        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_u1084a", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_u1084a is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
        */
    }

    digitizer_u1084a::~digitizer_u1084a()
    {
        if( f_allocated )
        {
            ViStatus t_result;
/*
            MTINFO( mtlog, "deallocating dma buffer..." );

            iterator t_it( f_buffer );
            for( size_t index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }

            MTINFO( mtlog, "disconnecting from digitizer card..." );

            t_result = DisconnectFromDevicePX4( f_handle );
            if( t_result != VI_SUCCESS )
            {
                PrintU1084AError( f_handle, t_result, "failed to disconnect from digitizer card: " );
                exit( -1 );
            }
            */
        }
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
        */
    }

    bool digitizer_u1084a::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        ViStatus t_result;
/*
        MTINFO( mtlog, "connecting to digitizer card..." );

        t_result = ConnectToDevicePX4( &f_handle, 1 );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to connect to digitizer card: " );
            return false;
        }

        //MTINFO( mtlog, "setting power up defaults..." );

        t_result = SetPowerupDefaultsPX4( f_handle );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to enter default state: " );
            return false;
        }

        MTINFO( mtlog, "allocating dma buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                block* t_new_block = new block();
                t_result = AllocateDmaBufferPX4( f_handle, f_buffer->record_size(), t_new_block->handle() );
                if( t_result != VI_SUCCESS )
                {
                    std::stringstream t_buff;
                    t_buff << "failed to allocate dma memory for block " << index <<": ";
                    PrintU1084AError( f_handle, t_result, t_buff.str().c_str() );
                    return false;
                }
                t_new_block->set_data_size( f_buffer->record_size() );
                t_new_block->set_data_nbytes( f_buffer->record_size() );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }
*/
        f_allocated = true;
        return true;
    }
    bool digitizer_u1084a::initialize( request* a_request )
    {
        ViStatus t_result;
/*
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
            if( t_result != VI_SUCCESS )
            {
                PrintU1084AError( f_handle, t_result, "failed to activate channel 1: " );
                return false;
            }

        }
        if( a_request->mode() == request_mode_t_dual_separate || a_request->mode() == request_mode_t_dual_interleaved )
        {
            t_result = SetActiveChannelsPX4( f_handle, PX4CHANSEL_DUAL_1_2 );
            if( t_result != VI_SUCCESS )
            {
                PrintU1084AError( f_handle, t_result, "failed to activate channels 1 and 2: " );
                return false;
            }
        }

        //MTINFO( mtlog, "setting clock rate..." );

        t_result = SetInternalAdcClockRatePX4( f_handle, a_request->rate() );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set clock rate: " );
            return false;
        }
*/
        return true;
    }
    void digitizer_u1084a::execute()
    {
        iterator t_it( f_buffer, "dig-u1084a" );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        //MTINFO( mtlog, "waiting" );
/*
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
*/
        return;
    }
    void digitizer_u1084a::cancel()
    {
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        return;
    }
    void digitizer_u1084a::finalize( response* a_response )
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

    bool digitizer_u1084a::start()
    {/*
        ViStatus t_result = BeginBufferedPciAcquisitionPX4( f_handle, PX4_FREE_RUN );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to begin dma acquisition: " );
            return false;
        }
*/
        return true;
    }
    bool digitizer_u1084a::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
/*
        ViStatus t_result = GetPciAcquisitionDataFastPX4( f_handle, f_buffer->record_size(), a_block->data_bytes(), 0 );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to acquire dma data over pci: " );
            t_result = EndBufferedPciAcquisitionPX4( f_handle );
            return false;
        }
*/
        // the timestamp is acquired after the data is transferred to avoid the problem on the u1084a where
        // the first record can take unusually long to be acquired.
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) - f_start_time );

        ++f_record_count;

        return true;
    }
    bool digitizer_u1084a::stop()
    {/*
        ViStatus t_result = EndBufferedPciAcquisitionPX4( f_handle );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to end dma acquisition: " );
            return false;
        }
        ++f_acquisition_count;*/
        return true;
    }

    bool digitizer_u1084a::write_mode_check( request_file_write_mode_t mode )
    {
        return true;
    }

    unsigned digitizer_u1084a::data_type_size()
    {
        return digitizer_u1084a::s_data_type_size;
    }


    bool digitizer_u1084a::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_u1084a::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }


    //***********************************
    // Block Cleanup u1084a
    //***********************************

    block_cleanup_u1084a::block_cleanup_u1084a( byte_type* a_data ) :
        f_triggered( false ),
        f_data( a_data )
    {}
    block_cleanup_u1084a::~block_cleanup_u1084a()
    {}
    bool block_cleanup_u1084a::delete_data()
    {
        if( f_triggered ) return true;
        delete [] f_data;
        f_triggered = true;
        return true;
    }


    //***********************************
    // test_digitizer_u1084a
    //***********************************

    bool test_digitizer_u1084a::run_test()
    {
        ViStatus t_result;
        ViSession f_handle;

        MTINFO( mtlog, "beginning allocation phase" );

        MTDEBUG( mtlog, "connecting to digitizer card..." );

        ViString options = "";
        // for a simulated device:
        //ViString options = "simulate=1";
        t_result = Acqrs_InitWithOptions( "PCI::INSTR0", VI_FALSE, VI_FALSE, options, &f_handle );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to connect to digitizer card: " );
            return false;
        }

        MTDEBUG( mtlog, "allocating memory buffer..." );

        block* t_block = NULL;
        // prog guide, pg 24: should be multiple of 32 (16) for single (dual) channel acquisition
        unsigned t_rec_size = 16384;

        try
        {
            // the +16 in the block size is recommended in the programmer's reference, pg
            t_block = block::allocate_block< digitizer_u1084a::data_type >( t_rec_size + 16 );
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

        // acquisition mode: 7 == SSR (Sustained Sequential Recording) mode
        ViInt32 acqMode = 7;

        // must set everything in the readParams
        AqReadParameters readParams;
        readParams.dataType = 0; // 1 byte, signed
        readParams.readMode = acqMode; // gated data from an SSR
        readParams.firstSegment = 0; //
        readParams.nbrSegments = 1; //
        readParams.firstSampleInSeg = 0; // typically 0
        readParams.nbrSamplesInSeg = t_rec_size; // requested number of samples
        readParams.segmentOffset = 0; // not used for readMode == 7
        readParams.dataArraySize = t_rec_size; // # of bytes in the output array
        readParams.segDescArraySize = sizeof( AqSegmentDescriptor ); // # of bytes in the segDescArray
        readParams.flags = 0; // with bit 2 == 0, data is reset after being read
        readParams.reserved = readParams.reserved2 = readParams.reserved3 = 0;

        MTDEBUG( mtlog, "setting run configuration..." );

        double clock_rate = 200.; // clock rate in MHz
        double sample_interval = 1. / ( clock_rate * 1.e6 ); // sampling interval in seconds
        t_result = AcqrsD1_configHorizontal( f_handle, sample_interval, 0 ); // delay time (last param) is overwritten later
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set horizontal settings: " );
            return false;
        }

        t_result = AcqrsD1_configVertical( f_handle, 1, u1084a_range, fabs(u1084a_min_val), 0, 0 );

        ViInt32 flags = 0; // pg 91 of the programmer's guide; maybe should be 10?
        t_result = AcqrsD1_configMode( f_handle, acqMode, 0 /*unused*/, 0 );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set acquisition mode: " );
            return false;
        }

        t_result = AcqrsD1_configAvgConfig( f_handle, 0, "NbrSamples", &(readParams.nbrSamplesInSeg) );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set number of samples: " );
            return false;
        }

        t_result = AcqrsD1_configAvgConfig( f_handle, 0, "NbrSegments", &(readParams.nbrSegments) );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set number of segments: " );
            return false;
        }

        ViInt32 startDelay = 32; // prog guide, pg 24: should be multiple of 32 (16) for single (dual) channel acquisition
                                 // if 0, then the first 10 (5) samples will be 0
        t_result = AcqrsD1_configAvgConfig( f_handle, 0, "StartDelay", &startDelay );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set start delay: " );
            return false;
        }

        ViInt32 stopDelay = 0;
        t_result = AcqrsD1_configAvgConfig( f_handle, 0, "StopDelay", &stopDelay );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set stop delay: " );
            return false;
        }

        MTINFO( mtlog, "initialization complete!\n" );


        MTINFO( mtlog, "beginning run phase" );

        MTDEBUG( mtlog, "beginning acquisition" );

        t_result = AcqrsD1_acquire( f_handle );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to begin acquisition: " );
            return false;
        }

        MTDEBUG( mtlog, "acquiring a record" );

        ViInt32 timeout = 1000; // ms
        AqDataDescriptor dataDesc; // data descriptor
        AqSegmentDescriptor segDesc[1]; // segment descriptor

        t_result = AcqrsD1_processData( f_handle, 0, 1 );
        t_result = AcqrsD1_waitForEndOfProcessing( f_handle, timeout );
        t_result = AcqrsD1_readData( f_handle, 1, &readParams, t_block->data_bytes(), &dataDesc, segDesc );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to acquire dma data over pci: " );
            t_result = AcqrsD1_stopAcquisition( f_handle );
            return false;
        }

        MTDEBUG( mtlog, "ending acquisition..." );

        t_result = AcqrsD1_stopAcquisition( f_handle );
        if( t_result != VI_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to end dma acquisition: " );
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
