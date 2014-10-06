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

// to disable the "conversion from string literals to char* is deprecated" warning
#pragma GCC diagnostic ignored "-Wwrite-strings"

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
        AgMD1_error_message( a_handle, a_status, t_buff );
        if( a_status > 0 )
        {
            MTWARN( mtlog, a_prepend_msg << t_buff << " (status code: " << a_status << ")" );
        }
        else
        {
            MTERROR( mtlog, a_prepend_msg << t_buff << " (status code: " << a_status << ")" );
        }
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


/*
        // Edit resource and options as needed.  resource is ignored if option Simulate=true
        char resource[] = "PCI::INSTR0";

        // If desired, use 'DriverSetup= CAL=0' to prevent digitizer from doing a SelfCalibration each time
        // it is initialized or reset which is the default behavior.
        char options[]  = "DriverSetup=CAL=0";

        ViBoolean t_idQuery = VI_TRUE;
        ViBoolean t_reset   = VI_TRUE;

        // Initialize the driver.  See driver help topic "Initializing the IVI-C Driver" for additional information
        t_result = AgMD1_InitWithOptions( resource, t_idQuery, t_idQuery, options, &f_handle );
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to initialize the digitizer card: " );
            return false;
        }
        MTINFO( mtlog, "Driver initialized" );
*/

        ViInt32 numInstr; // Number of instruments
        t_result = AcqrsD1_multiInstrAutoDefine("", &numInstr);
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "autoDefine error: " << errMsg );
        }
        if (numInstr < 1)
        {
            MTERROR( mtlog, "found no instruments!");
            return false;
        }
        ViChar rscStr[16] = "PCI::INSTR0"; // resource string
        ViChar options[32] = ""; //no options needed

        MTDEBUG( mtlog, "found " << numInstr << " devices" );

        // prog guide, pg 24: should be multiple of 32 (16) for single (dual) channel acquisition
        unsigned t_rec_size = 8024;
        // the +16 in the block size is recommended in the programmer's reference, pg
        unsigned t_dig_pts = t_rec_size + 16;


        MTINFO( mtlog, "beginning initialization phase" );
        t_result = Acqrs_InitWithOptions(rscStr, VI_FALSE, VI_FALSE, options, &f_handle);
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "Init with options error: " << errMsg << "\n" );
            return false;
        }
/*
        // Read and output a few attributes
        // Note: status should be checked after each driver call but is omitted here for clarity.
        ViChar t_str_buff[128];
        ViBoolean t_simulate;
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_SPECIFIC_DRIVER_PREFIX, 127, t_str_buff );
        MTINFO( mtlog, "DRIVER_PREFIX: " << t_str_buff);
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_SPECIFIC_DRIVER_REVISION, 127, t_str_buff );
        MTINFO( mtlog, "DRIVER_REVISION: " <<  t_str_buff);
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_SPECIFIC_DRIVER_VENDOR, 127, t_str_buff );
        MTINFO( mtlog, "DRIVER_VENDOR: " <<  t_str_buff);
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_SPECIFIC_DRIVER_DESCRIPTION, 127, t_str_buff );
        MTINFO( mtlog, "DRIVER_DESCRIPTION: " <<  t_str_buff);
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_INSTRUMENT_MODEL, 127, t_str_buff );
        MTINFO( mtlog, "INSTRUMENT_MODEL: " <<  t_str_buff);
        t_result = AgMD1_GetAttributeViString(f_handle, "", AGMD1_ATTR_INSTRUMENT_FIRMWARE_REVISION, 127, t_str_buff );
        MTINFO( mtlog, "FIRMWARE_REVISION: " <<  t_str_buff);
        t_result = AgMD1_GetAttributeViBoolean(f_handle, "", AGMD1_ATTR_SIMULATE, &t_simulate );
        if( t_simulate == VI_TRUE )
        {
            MTINFO( mtlog, "SIMULATE:           True\n\n");
        }
        else
        {
            MTINFO( mtlog, "SIMULATE:           False\n\n");
        }
*/

        MTDEBUG( mtlog, "setting run configuration..." );

        // Setup acquisition - Records must be 1 for Channel.Measurement methods.
        // For multiple records use Channel.MutiRecordMeasurement methods.

        // Config timebase
        double t_clock_rate = 250.; // clock rate in MHz
        ViReal64 t_sample_interval = 1. / ( t_clock_rate * 1.e6 ); // sampling interval in seconds
        t_result = AcqrsD1_configHorizontal( f_handle, t_sample_interval, 0.0 );
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "horizontal conf error: " << errMsg << "\n" );
            return false;
        }

        // Config for SAR
        t_result = AcqrsD1_configMode( f_handle, 0, 0, 10); //10 -> SAR
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "problem setting SAR: " << errMsg << "\n" );
            return false;
        }

        // Config sampling
        ViInt32 t_number_samples = 25000000;
        ViInt32 t_number_segments = 1;
        ViInt32 t_number_banks = 2; // must be 2 for the u1084a in SAR
        t_result = AcqrsD1_configMemoryEx( f_handle, 0, t_number_samples, t_number_segments, t_number_banks, 0);
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "problem config memory: " << errMsg << "\n" );
            return false;
        }

        // Config vertical settings Ch 1
        ViReal64 t_full_scale = 1.0; // volts
        ViReal64 t_offset = 0.0; // volts
        ViInt32 t_coupling = 3; // 3 is for DC coupling
        ViInt32 t_bandwidth = 0; // o is for no limit
        t_result = AcqrsD1_configVertical( f_handle, 1, t_full_scale, t_offset, t_coupling, t_bandwidth);
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "vertical conf error: " << errMsg << "\n" );
            return false;
        }

        // Config a trigger as edge on channel 1
        t_result = AcqrsD1_configTrigClass(f_handle, 0, 0x00000001, 0, 0, 0.0, 0.0);
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "trig class conf error: " << errMsg << "\n" );
            return false;
        }
        // Config trigger conditions
        ViInt32 t_trigger_coupling = 0; //0 for DC
        ViInt32 t_trigger_slope = 0; //0 for positive
        ViReal64 t_trigger_level = 0.0; //in % of full vertical scale
        t_result = AcqrsD1_configTrigSource(f_handle, 1, t_trigger_coupling, t_trigger_slope, t_trigger_level, 0.0);
        if ( t_result )
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "trig source conf error: " << errMsg << "\n" );
            return false;
        }

/*        t_result = AgMD1_ConfigureAcquisition( f_handle, 1, t_rec_size, t_sample_interval ); // Records, PointsPerRecord, SampleRate
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed when making acquisition settings: " );
            / *
            if( t_result == ACQIRIS_WARN_SETUP_ADAPTED )
            {
                MTWARN( mtlog, "tried to set: " << sample_interval << ", 0" );
                ViReal64 set_si, set_delay;
                AcqrsD1_getHorizontal( f_handle, &set_si, &set_delay );
                MTWARN( mtlog, "setting applied: " << set_si << ", " << set_delay );
            }* /
            return false;
        }
        t_result = AgMD1_ConfigureChannel( f_handle, "Channel1", 2.0, 0.0, AGMD1_VAL_TRIGGER_COUPLING_DC, VI_TRUE ); // Range, Offset, Coupling, Enabled
        //t_result = AgMD1_ConfigureChannel(f_handle, "Channel1", u1084a_range, fabs(u1084a_min_val), AGMD1_VAL_TRIGGER_COUPLING_DC, VI_TRUE); // Range, Offset, Coupling, Enabled
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set input settings: " );
            return false;
        }

        // Setup triggering
        t_result = AgMD1_SetAttributeViString( f_handle, "", AGMD1_ATTR_ACTIVE_TRIGGER_SOURCE, "External1" );
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set trigger source: " );
            return false;
        }

        t_result = AgMD1_SetAttributeViInt32( f_handle, "External1", AGMD1_ATTR_TRIGGER_TYPE, AGMD1_VAL_IMMEDIATE_TRIGGER ); // No trigger required
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to set trigger type: " );
            return false;
        }

*/
        MTINFO( mtlog, "initialization complete!\n" );

        MTDEBUG( mtlog, "allocating memory buffer..." );
        block* t_block = NULL;

        try
        {
            t_block = block::allocate_block< digitizer_u1084a::data_type >( t_dig_pts );
            t_block->set_data_size( t_rec_size );
            t_block->set_data_nbytes( t_rec_size );
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "allocation complete!\n" );

        MTINFO( mtlog, "initialization complete!\n" );


        MTINFO( mtlog, "beginning run phase" );

        MTDEBUG( mtlog, "beginning acquisition" );


        // Calibrate, initiate measurement, and read the waveform data
        /*
        MTINFO( mtlog, "Calibrating...");
        t_result = AgMD1_SelfCalibrate(f_handle);
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed calibration: " );
            delete t_block;
            return false;
        }*/

    /*
        ViInt64 ActualPoints;
        ViInt64 FirstValidPoint;
        ViReal64 InitialXOffset;
        ViReal64 InitialXTimeSeconds;
        ViReal64 InitialXTimeFraction;
        ViReal64 XIncrement;
        ViReal64 ScaleFactor;
        ViReal64 ScaleOffset;
    */
        t_result = AcqrsD1_acquire( f_handle );
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "start acquisition error: " << errMsg << "\n" );
            return false;
        }

        MTINFO( mtlog, "Measuring Waveform on Channel1...");
        ViInt32 t_timeout = 1000; // ms

        // ensure there was a trigger and that acquisition is complete
        MTDEBUG( mtlog, "first wait for acquisition to complete");
        t_result = AcqrsD1_waitForEndOfAcquisition( f_handle, t_timeout );
        if (t_result != VI_SUCCESS)
        {
            MTWARN( mtlog, "acquisition never triggered, forcing" );
            t_result = AcqrsD1_forceTrigEx( f_handle, 1, 0, 0 ); //SAR requires type 1
            if (t_result)
            {
                ViChar errMsg[512] = "";
                Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
                MTERROR( mtlog, "force trig error: " << errMsg << "\n" );
                return false;
            }
            t_result = AcqrsD1_waitForEndOfAcquisition( f_handle, t_timeout );
            if (t_result)
            {
                MTWARN( mtlog, "acquisition still times out after forced trigger");
            }
        }

        //setup read
        AqReadParameters readPar;
        readPar.dataType = ReadInt8; //8bit, raw ADC values data type
        readPar.readMode = ReadModeStdW; // Single-segment read mode
        readPar.firstSegment = 0;
        readPar.nbrSegments = 1;
        readPar.firstSampleInSeg = 0;
        readPar.nbrSamplesInSeg = t_number_samples;
        readPar.segmentOffset = 0;
        readPar.dataArraySize = (t_number_samples + 32) * sizeof(ViInt8); //Array size in bytes
        readPar.segDescArraySize = sizeof(AqSegmentDescriptor);
        readPar.flags = 0;
        readPar.reserved = 0;
        readPar.reserved2 = 0;
        readPar.reserved3 = 0;
        AqDataDescriptor dataDesc;
        AqSegmentDescriptor segDesc;
        ViInt8 *adcArrayP = new ViInt8[readPar.dataArraySize];

        //t_result = AcqrsD1_readData( f_handle, 1, &readPar, adcArrayP, &dataDesc, &segDesc);
        MTDEBUG (mtlog, "then read data");
        t_result = AcqrsD1_readData( f_handle, 1, &readPar, adcArrayP, &dataDesc, &segDesc);
        //t_result = AcqrsD1_readData( f_handle, 1, &readPar, reinterpret_cast< ViInt8* > (t_block->data_bytes()), &dataDesc, &segDesc);
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "read data error: " << errMsg << "\n" );
            return false;
        }
/*        t_result = AgMD1_ReadWaveformInt8( f_handle, "Channel1", t_timeout, t_dig_pts, (char*)(t_block->data_bytes()), &ActualPoints, &FirstValidPoint,
                &InitialXOffset, &InitialXTimeSeconds, &InitialXTimeFraction, &XIncrement, &ScaleFactor, &ScaleOffset );
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed waveform acquisition: " );
            delete t_block;
            return false;
        }
*/
/*        MTDEBUG( mtlog, "and free the bank");
        t_result = AcqrsD1_freeBank( f_handle, 0 );
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "free bank error: " << errMsg << "\n" );
            return false;
        }*/
        
        MTDEBUG( mtlog, "ending acquisition..." );
        t_result = AcqrsD1_stopAcquisition( f_handle);
        if (t_result)
        {
            ViChar errMsg[512] = "";
            Acqrs_errorMessage(VI_NULL, t_result, errMsg, 512);
            MTERROR( mtlog, "problem ending acquisition: " << errMsg << "\n" );
            return false;
        }

/*
        // Close the driver
        t_result = AgMD1_close( f_handle );
        if( t_result != AGMD1_SUCCESS )
        {
            PrintU1084AError( f_handle, t_result, "failed to close: " );
            delete t_block;
            return false;
        }
*/


        std::stringstream t_block_str;
        std::stringstream adc_str;
        for( unsigned i = 0; i < 99; ++i )
        {
            t_block_str << int(t_block->data_bytes()[ i ]) << ", ";
            adc_str << int(adcArrayP[ i ]) << ", ";
        }
        t_block_str << t_block->data_bytes()[ 99 ];
        adc_str << int(adcArrayP[ 99 ]);
        MTDEBUG( mtlog, "the first 100 samples taken (in t_block):\n" << t_block_str.str() );
        MTDEBUG( mtlog, "the first 100 samples taken (in ViInt8):\n" << adc_str.str() );

        MTINFO( mtlog, "run complete!\n" );


        MTINFO( mtlog, "beginning finalization phase" );

        MTDEBUG( mtlog, "deallocating memory buffer" );

        delete t_block;

        MTINFO( mtlog, "finalization complete!\n" );


        return true;
    }

}
