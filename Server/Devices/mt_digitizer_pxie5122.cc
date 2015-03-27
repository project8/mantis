#define MANTIS_API_EXPORTS

#include "mt_digitizer_pxie5122.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"
#include "mt_param.hh"

#include "M3Types.hh"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_pxie5122" );

    MT_REGISTER_DIGITIZER( digitizer_pxie5122, "pxie5122" );


    void digitizer_pxie5122_config_template::add( param_node* a_node, const std::string& a_type )
    {
        param_node* t_new_node = new param_node();
        t_new_node->add( "resource-name", param_value() << "PXI1Slot2" ); // Real digitizer: PXI1Slot2; Simulated digitizer: Dev1
        t_new_node->add( "rate-req", param_value() << 100 );
        t_new_node->add( "n-channels", param_value() << 1 );
        t_new_node->add( "data-mode", param_value() << monarch3::sDigitizedS );
        t_new_node->add( "channel-mode", param_value() << monarch3::sSeparate );
        t_new_node->add( "sample-size", param_value() << 1 );
        t_new_node->add( "buffer-size", param_value() << 512 );
        t_new_node->add( "record-size-req", param_value() << 524288 );// 1048576 );
        t_new_node->add( "data-chunk-size", param_value() << 1024 );
        t_new_node->add( "input-impedance", param_value() << 50 );
        t_new_node->add( "voltage-range", param_value() << 0.5 );
        t_new_node->add( "voltage-min", param_value() << -0.25 );
        t_new_node->add( "input-coupling", param_value() << 1 ); // DC coupling
        t_new_node->add( "probe-attenuation", param_value() << 1.0 );
        t_new_node->add( "acq-timeout", param_value() << 10.0 );
        a_node->add( a_type, t_new_node );

    }

    const unsigned digitizer_pxie5122::s_data_type_size = sizeof( digitizer_pxie5122::data_type );
    unsigned digitizer_pxie5122::data_type_size()
    {
        return digitizer_pxie5122::s_data_type_size;
    }

    digitizer_pxie5122::digitizer_pxie5122() :
            //f_semaphore( NULL ),
            f_handle(),
            f_resource_name(),
            f_allocated( false ),
            f_acq_timeout( 0. ),
            f_waveform_info(),
            f_start_time( 0 ),
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
        f_semaphore = sem_open( "/digitizer_pxie5122", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_pxie5122 is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
        */
    }

    digitizer_pxie5122::~digitizer_pxie5122()
    {
        if( f_allocated ) deallocate();
     
        if( f_handle ) handle_error( niScope_close( f_handle ) );
        
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
        */
    }

    bool digitizer_pxie5122::allocate()
    {
        if( f_allocated )
        {
            MTERROR( mtlog, "Buffer is already allocated" );
            return false;
        }

        if( f_buffer == NULL )
        {
            MTERROR( mtlog, "Buffer must be created before allocation" );
            return false;
        }

        MTINFO( mtlog, "Allocating buffer" );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                block* t_new_block = block::allocate_block< data_type >( f_buffer->block_size() );
                t_new_block->set_cleanup( new block_cleanup_pxie5122( t_new_block->data_bytes() ) );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "Unable to allocate buffer: " << e.what() );
            return false;
        }

        f_allocated = true;
        return true;
    }

    bool digitizer_pxie5122::deallocate()
    {
        if( !f_allocated )
        {
            MTWARN( mtlog, "Buffer is not allcoated" );
            return true;
        }

        MTINFO( mtlog, "Deallocating buffer" );

        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            f_buffer->delete_block( index );
        }
        f_allocated = false;
        return true;
    }

    bool digitizer_pxie5122::initialize( param_node* a_global_config, param_node* a_dev_config )
    {
        //MTINFO( mtlog, "resetting counters..." );

        //MTINFO( mtlog, "initializing the digitizer" );

        //TODO: allow multiple channels
        // for now: enforce one channel
        if( a_dev_config->get_value< unsigned >( "n-channels", 1 ) != 1 )
        {
            MTERROR( mtlog, "For now you can only use 1 channel.  Sorry" );
            return false;
        }

        // Check data mode and channel mode
        uint32_t t_data_mode = a_dev_config->get_value< uint32_t >( "data-mode" );
        if( t_data_mode != monarch3::sDigitizedUS && t_data_mode != monarch3::sDigitizedS )
        {
            MTERROR( mtlog, "Data can only be taken in <digitized> mode" );
            return false;
        }
        if( a_dev_config->get_value< uint32_t >( "channel-mode" ) != monarch3::sSeparate )
        {
            MTERROR( mtlog, "Multi-channel data can only be recorded in <separate> mode" );
            return false;
        }

        std::string resourceNameStr = a_dev_config->get_value( "resource-name", "" );
        MTDEBUG( mtlog, "Resource name from config: <" << resourceNameStr << ">" );
        if( resourceNameStr.empty() )
        {
            MTERROR( mtlog, "No resource name was provided" );
            return false;
        }
        if( ! f_resource_name.empty() && resourceNameStr != f_resource_name )
        {
            MTERROR( mtlog, "Resource name must match previously used name: " << f_resource_name );
            return false;
        }

        if( ! f_handle )
        {
            f_resource_name = resourceNameStr;
            MTDEBUG( mtlog, "Connecting to the 5122 using resource name <" << f_resource_name << ">" );

            //ViChar* resourceName = new ViChar[ f_resource_name.size() ];
            //strcpy( resourceName, f_resource_name.c_str() );
            if( ! handle_error( niScope_init( const_cast< char* >( f_resource_name.c_str() ), NISCOPE_VAL_FALSE, NISCOPE_VAL_FALSE, &f_handle ) ) )
            {
                //delete[] resourceName;
                return false;
            }
            //delete[] resourceName;
        }

        MTDEBUG( mtlog, "Configuring the 5122" );
        // call to niScpe_ConfigureChanCharacteristics
        // input impedance may be either 50, or 1000000
        unsigned t_impedance = a_dev_config->get_value< unsigned >( "input-impedance", 50 );
        if( t_impedance != 50 && t_impedance != 1000000 )
        {
            MTERROR( mtlog, "Input impedance must be either 50 Ohms or 1000000 Ohms; value provided: " << t_impedance );
            return false;
        }
        // for now just use -1 for max input frequency
        if( ! handle_error( niScope_ConfigureChanCharacteristics( f_handle, "1", t_impedance, -1 ) ) )
        {
            return false;
        }

        // call to niScope_ConfigureHorizontalTiming
        // TODO: the block size request assumes that we're only using 1 channel
        // Note that the record size request is passed as the 3rd parameter; this is correct regardless of the number of channels in use;
        // This parameter in the NI function is the minimum number of samples in the record for each channel according to the NI-SCOPE documentation.
        // Must convert MHz rate request to Hz for NI-SCOPE
        if( ! handle_error( niScope_ConfigureHorizontalTiming( f_handle, a_dev_config->get_value< double >( "rate-req" ) * 1.e6,
            a_dev_config->get_value< unsigned >( "record-size-req" ), 0, 1, VI_TRUE ) ) )
        {
            return false;
        }
        ViReal64 t_actual_rate;
        if( ! handle_error( niScope_SampleRate( f_handle, &t_actual_rate ) ) )
        {
            return false;
        }
        // convert from Hz to MHz
        t_actual_rate *= 1.e-6;
        a_dev_config->replace( "rate", param_value() << t_actual_rate );
        ViInt32 t_actual_rec_size;
        if( ! handle_error( niScope_ActualRecordLength( f_handle, &t_actual_rec_size ) ) )
        {
            return false;
        }
        a_dev_config->replace( "record-size", param_value() << t_actual_rec_size );
        MTINFO( mtlog, "Actual rate: " << t_actual_rate << " MHz; Actual record size: " << t_actual_rec_size );

        // check buffer allocation
        // this section assumes 1 channel, in not multiplying t_actual_rec_size by the number of channels when converting to block size
        bool t_must_allocate = false; // will be done later, assuming the initialization succeeds
        unsigned t_buffer_size = a_dev_config->get_value< unsigned >( "buffer-size", 512 );
        if( f_buffer != NULL && ( f_buffer->size() != t_buffer_size || f_buffer->block_size() != t_actual_rec_size ) )
        {
            // need to redo the buffer
            if( f_allocated ) deallocate();
            delete f_buffer;
            f_buffer = NULL;
        }
        if( f_buffer == NULL )
        {
            t_must_allocate = true;
            f_buffer = new buffer( t_buffer_size, t_actual_rec_size );
        }

        // call to niScope_ConfigureVertical
        ViReal64 t_voltage_range = a_dev_config->get_value< ViReal64 >( "voltage-range", 0.5 );
        ViReal64 t_voltage_min = a_dev_config->get_value< ViReal64 >( "voltage-min", -0.5 * t_voltage_range );
        // the voltage offset is the center of the voltage range chosen
        ViReal64 t_voltage_offset = t_voltage_min + 0.5 * t_voltage_range;
        ViInt32 t_coupling = a_dev_config->get_value< ViInt32 >( "input-coupling", NISCOPE_VAL_AC );
        if( t_coupling != NISCOPE_VAL_AC && t_coupling != NISCOPE_VAL_DC && t_coupling != NISCOPE_VAL_GND )
        {
            MTERROR( mtlog, "Invalid input coupling: " << t_coupling );
            return false;
        }
        ViReal64 t_probe_attenuation = a_dev_config->get_value< ViReal64 >( "probe-attenuation", 1. );
        if( t_probe_attenuation < 0 )
        {
            MTERROR( mtlog, "Probe attenuation must be a real, positive number" );
            return false;
        }
        if( ! handle_error( niScope_ConfigureVertical( f_handle, "1", t_voltage_range, t_voltage_offset, t_coupling, t_probe_attenuation, true ) ) )
        {
            return false;
        }

        // get the scaling coefficients
        ViInt32 t_n_coeff_sets; // first determine the size of the array used to store the scaling coefficients
        if( ! handle_error( niScope_GetScalingCoefficients( f_handle, "1", 0, NULL, &t_n_coeff_sets ) ) )
        {
            return false;
        }
        niScope_coefficientInfo* t_coeff_info_array = new niScope_coefficientInfo[ t_n_coeff_sets ];
        if( !handle_error( niScope_GetScalingCoefficients( f_handle, "1", t_n_coeff_sets, t_coeff_info_array, &t_n_coeff_sets ) ) )
        {
            return false;
        }
        get_calib_params2( 14 /*bit depth*/, s_data_type_size, t_voltage_min, t_voltage_range, t_coeff_info_array[0].gain, &f_params );
        a_dev_config->replace( "voltage-min", param_value() << f_params.v_min );
        a_dev_config->replace( "voltage-range", param_value() << f_params.v_range );
        a_dev_config->replace( "dac-gain", param_value() << f_params.dac_gain );

        // call to niScope_ConfigureTriggerSoftware to allow for continuous acquisition
        if( ! handle_error( niScope_ConfigureTriggerSoftware( f_handle, 0., 0. ) ) )
        {
            return false;
        }

        // tell the digitizer to fetch each record starting at the read pointer, which gets moved after each fetch
        if( ! handle_error( niScope_SetAttributeViInt32( f_handle, VI_NULL, NISCOPE_ATTR_FETCH_RELATIVE_TO, NISCOPE_VAL_READ_POINTER ) ) )
        {
            return false;
        }

        // get the acquisition timeout
        f_acq_timeout = a_dev_config->get_value< double >( "timeout", -1. );

        // allocate the buffer if needed
        if( t_must_allocate )
        {
            allocate();
        }

        f_record_last = ( record_id_type )( ceil( ( double )( a_dev_config->get_value< double >( "rate" ) * a_global_config->node_at( "run" )->get_value< double >( "duration" ) * 1.e3 ) / ( double )( f_buffer->block_size() ) ) );
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        return true;
    }

    void digitizer_pxie5122::execute()
    {
        iterator t_it( f_buffer, "dig_pxie5122" );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        //MTINFO( mtlog, "waiting" );

        f_buffer_condition->wait();

        MTINFO( mtlog, "Digitizer loose at <" << t_it.index() << ">" );

        // nsoblath, 1/30/15: why did i have this here before?
        //int t_old_cancel_state;
        //pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

        //start acquisition
        if( start() == false )
        {
            return;
        }

        MTINFO( mtlog, "Planning on " << f_record_last << " records" );

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
                    MTINFO( mtlog, "Digitizer was canceled mid-run" );
                    f_cancel_condition.release();
                }
                else
                {
                    MTINFO( mtlog, "Finished normally" );
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

                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                MTINFO( mtlog, "Finished abnormally because acquisition failed" );

                return;
            }

            //MTDEBUG( mtlog, "digitizer_pxie5122:" );
            //f_buffer->print_states();

            t_it->set_acquired();

            if( +t_it == false )
            {
                MTINFO( mtlog, "Blocked at <" << t_it.index() << ">" );

                //stop live timer
                get_time_monotonic( &t_live_stop_time );

                //accumulate live time
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                if( stop() == false )
                {
                    //GET OUT
                    MTINFO( mtlog, "Finished abnormally because halting streaming failed" );
                    return;
                }

                //start dead timer
                get_time_monotonic( &t_dead_start_time );

                //wait
                f_buffer_condition->wait();

                //stop dead timer
                get_time_monotonic( &t_dead_stop_time );

                //accumulate dead time
                f_dead_time += time_to_nsec( t_dead_stop_time ) - time_to_nsec( t_dead_start_time );

                //start acquisition
                if( start() == false )
                {
                    // to make sure we don't deadlock anything
                    if( f_cancel_condition.is_waiting() )
                    {
                        f_cancel_condition.release();
                    }

                    //GET OUT
                    MTINFO( mtlog, "Finished abnormally because starting streaming failed" );
                    return;
                }

                //increment block (waits for mutex lock)
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                MTINFO( mtlog, "Loose at <" << t_it.index() << ">" );
            }
            //MTINFO( mtlog, "record count: " << f_record_count );

        }

        return;
    }

    /* Asyncronous cancelation:
    Main execution loop checks for f_canceled, and exits if it's true.
    */
    void digitizer_pxie5122::cancel()
    {
        MTDEBUG(mtlog, "Canceling digitizer test");
        //cout << "CANCELLING DIGITIZER TEST" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  digitizer_pxie5122 is done canceling" );
        return;
    }

    void digitizer_pxie5122::finalize( param_node* a_response )
    {
        //MTINFO( mtlog, "calculating statistics..." );
        double t_livetime = (double) (f_live_time) * SEC_PER_NSEC;
        double t_deadtime = (double) f_dead_time * SEC_PER_NSEC;
        double t_mb_recorded = (double) (4 * f_record_count);

        param_node* t_resp_node = new param_node();
        param_value t_value;
        t_resp_node->add( "record-count", t_value << f_record_count );
        t_resp_node->add( "acquisition-count", t_value << f_acquisition_count );
        t_resp_node->add( "livetime", t_value << t_livetime );
        t_resp_node->add( "deadtime", t_value << t_deadtime );
        t_resp_node->add( "mb-recorded", t_value << t_mb_recorded );
        t_resp_node->add( "digitization-rate", t_value << t_mb_recorded / t_livetime );

        a_response->add( "digitizer-test", t_resp_node );

        return;
    }

    bool digitizer_pxie5122::start()
    {
        return handle_error( niScope_InitiateAcquisition( f_handle ) );
    }

    bool digitizer_pxie5122::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );

        if( !handle_error( niScope_FetchBinary16( f_handle, "1", f_acq_timeout, a_block->get_data_size(), (ViInt16*)a_block->data_bytes(), &f_waveform_info) ) )
        {
            return false;
        }

        // the timestamp is acquired after the data is transferred to avoid the problem on the px1500 where
        // the first record can take unusually long to be acquired.
        // it's done here too for consistency
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) - f_start_time );

        ++f_record_count;

        return true;
    }

    bool digitizer_pxie5122::stop()
    {
        ++f_acquisition_count;
        return handle_error( niScope_Abort( f_handle ) );
    }

    bool digitizer_pxie5122::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_pxie5122::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }

    bool digitizer_pxie5122::run_basic_test()
    {
        MTINFO( mtlog, "Beginning basic test of the PXIe5122" );

        MTINFO( mtlog, "Connecting to the digitizer" );

        // Resource name options:
        // - Real digitizer: PXI1Slot2
        // - Dummy (software) digitizer: Dev1
        std::string resourceNameStr( "PXI1Slot2" );
        MTDEBUG( mtlog, "Resource name from config: <" << resourceNameStr << ">" );
        if( resourceNameStr.empty() )
        {
            MTERROR( mtlog, "No resource name was provided" );
            return false;
        }
        if( ! f_resource_name.empty() && resourceNameStr != f_resource_name )
        {
            MTERROR( mtlog, "Resource name must match previously used name: " << f_resource_name );
            return false;
        }

        if( ! f_handle )
        {
            f_resource_name = resourceNameStr;
            MTDEBUG( mtlog, "Connecting to the 5122 using resource name <" << f_resource_name << ">" );

            //ViChar* resourceName = new ViChar[ f_resource_name.size() ];
            //strcpy( resourceName, f_resource_name.c_str() );
            if( ! handle_error( niScope_init( const_cast< char* >( f_resource_name.c_str() ), NISCOPE_VAL_FALSE, NISCOPE_VAL_FALSE, &f_handle ) ) )
            {
                //delete[] resourceName;
                return false;
            }
            //delete[] resourceName;
        }

        MTINFO( mtlog, "Connection successful" );

        MTINFO( mtlog, "Configuring the 5122" );
        // call to niScpe_ConfigureChanCharacteristics
        // input impedance may be either 50, or 1000000
        unsigned t_impedance = 50;
        if( t_impedance != 50 && t_impedance != 1000000 )
        {
            MTERROR( mtlog, "Input impedance must be either 50 Ohms or 1000000 Ohms; value provided: " << t_impedance );
            return false;
        }
        // for now just use -1 for max input frequency
        if( ! handle_error( niScope_ConfigureChanCharacteristics( f_handle, "1", t_impedance, -1 ) ) )
        {
            return false;
        }

        // call to niScope_ConfigureHorizontalTiming
        // TODO: the block size request assumes that we're only using 1 channel
        // Note that the record size request is passed as the 3rd parameter; this is correct regardless of the number of channels in use;
        // This parameter in the NI function is the minimum number of samples in the record for each channel according to the NI-SCOPE documentation.
        // Must convert MHz rate request to Hz for NI-SCOPE
        if( ! handle_error( niScope_ConfigureHorizontalTiming( f_handle, 100. * 1.e6,
                524288, 0, 1, VI_TRUE ) ) )
        {
            return false;
        }
        ViReal64 t_actual_rate;
        if( ! handle_error( niScope_SampleRate( f_handle, &t_actual_rate ) ) )
        {
            return false;
        }
        // convert from Hz to MHz
        t_actual_rate *= 1.e-6;
        ViInt32 t_actual_rec_size;
        if( ! handle_error( niScope_ActualRecordLength( f_handle, &t_actual_rec_size ) ) )
        {
            return false;
        }
        MTINFO( mtlog, "Actual rate: " << t_actual_rate << " MHz; Actual record size: " << t_actual_rec_size );

        // check buffer allocation
        // this section assumes 1 channel, in not multiplying t_actual_rec_size by the number of channels when converting to block size
        bool t_must_allocate = false; // will be done later, assuming the initialization succeeds
        unsigned t_buffer_size = 1;
        if( f_buffer != NULL && ( f_buffer->size() != t_buffer_size || f_buffer->block_size() != t_actual_rec_size ) )
        {
            // need to redo the buffer
            if( f_allocated ) deallocate();
            delete f_buffer;
            f_buffer = NULL;
        }
        if( f_buffer == NULL )
        {
            t_must_allocate = true;
            f_buffer = new buffer( t_buffer_size, t_actual_rec_size );
        }

        // call to niScope_ConfigureVertical
        ViReal64 t_voltage_range = 0.5;
        ViReal64 t_voltage_offset = 0.;
        ViInt32 t_coupling = NISCOPE_VAL_DC;
        if( t_impedance == 50 ) t_coupling = NISCOPE_VAL_DC; // 50-Ohm impedance requires DC coupling according to an error message from testing (3/16/17)
        if( t_coupling != NISCOPE_VAL_AC && t_coupling != NISCOPE_VAL_DC && t_coupling != NISCOPE_VAL_GND )
        {
            MTERROR( mtlog, "Invalid input coupling: " << t_coupling );
            return false;
        }
        ViReal64 t_probe_attenuation = 1.;
        if( t_probe_attenuation < 0 )
        {
            MTERROR( mtlog, "Probe attenuation must be a real, positive number" );
            return false;
        }
        if( ! handle_error( niScope_ConfigureVertical( f_handle, "1", t_voltage_range, t_voltage_offset, t_coupling, t_probe_attenuation, true ) ) )
        {
            return false;
        }

        // get the scaling coefficients
        ViInt32 t_n_coeff_sets;
        if( ! handle_error( niScope_GetScalingCoefficients( f_handle, "1", 0, NULL, &t_n_coeff_sets ) ) )
        {
            return false;
        }
        niScope_coefficientInfo* t_coeff_info_array = new niScope_coefficientInfo[ t_n_coeff_sets ];
        if( ! handle_error( niScope_GetScalingCoefficients( f_handle, "1", t_n_coeff_sets, t_coeff_info_array, &t_n_coeff_sets ) ) )
        {
            return false;
        }
        get_calib_params2( 14 /*bit depth*/, s_data_type_size, t_voltage_offset, t_voltage_range, t_coeff_info_array[0].gain, &f_params );
 
        // call to niScope_ConfigureTriggerSoftware to allow for continuous acquisition
        if( ! handle_error( niScope_ConfigureTriggerSoftware( f_handle, 0., 0. ) ) )
        {
            return false;
        }

        if( ! handle_error( niScope_SetAttributeViInt32( f_handle, VI_NULL, NISCOPE_ATTR_FETCH_RELATIVE_TO, NISCOPE_VAL_READ_POINTER ) ) )
        {
            return false;
        }

        // get the acquisition timeout
        f_acq_timeout = 10.;

        MTINFO( mtlog, "Configuration complete" );

        MTINFO( mtlog, "Allocating the buffer" );

        block* t_block = NULL;

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                t_block = block::allocate_block< data_type >( t_actual_rec_size );
                t_block->set_cleanup( new block_cleanup_pxie5122( t_block->data_bytes() ) );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "Unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "Allocation complete" );

        MTINFO( mtlog, "Beginning the run phase" );

        MTDEBUG( mtlog, "Starting acquisition" );

        if( ! handle_error( niScope_InitiateAcquisition( f_handle ) ) )
        {
            return false;
        }

        MTDEBUG( mtlog, "Acquiring a record (" << f_acq_timeout << ", " << t_block->get_data_size() << /* ", " << t_block->data_bytes() << */ ")" );

        if( ! handle_error( niScope_FetchBinary16( f_handle, "1", f_acq_timeout, t_block->get_data_size(), (ViInt16*)t_block->data_bytes(), &f_waveform_info) ) )
        {
            return false;
        }

        MTDEBUG( mtlog, "Stopping acquisition..." );

        if( ! handle_error( niScope_Abort( f_handle ) ) )
        {
            return false;
        }

        block_view< ViInt16 > t_block_view( t_block );
        std::stringstream t_str_buff;
        for( unsigned i = 0; i < 99; ++i )
        {
            t_str_buff << t_block_view.data_view()[ i ] << ", ";
        }
        t_str_buff << t_block->data_bytes()[ 99 ];
        MTDEBUG( mtlog, "The first 100 samples taken:\n" << t_str_buff.str() );

        MTINFO( mtlog, "Run phase complete!\n" );


        MTINFO( mtlog, "Deallocating buffer" );

        delete t_block;

        MTINFO( mtlog, "Deallocation complete!\n" );

        return true;
    }

    bool digitizer_pxie5122::handle_error( ViStatus a_status )
    {
        if( a_status == VI_SUCCESS ) return true;
        const unsigned t_buffer_size = 512;
        ViChar t_msg_buffer[ t_buffer_size ];
        niScope_GetErrorMessage( f_handle, a_status, t_buffer_size, t_msg_buffer );
        if( a_status > 0 )
        {
            MTWARN( mtlog, t_msg_buffer );
            return false;
        }
        else // a_status < 0, since VI_SUCCESS == 0
        {
            MTERROR( mtlog, t_msg_buffer );
            return false;
        }
    }


    //********************************
    // Block Cleanup -- Test Digitizer
    //********************************

    block_cleanup_pxie5122::block_cleanup_pxie5122( byte_type* a_memblock ) :
            block_cleanup(),
            f_triggered( false ),
            f_memblock( a_memblock )
    {}

    block_cleanup_pxie5122::~block_cleanup_pxie5122()
    {}

    bool block_cleanup_pxie5122::delete_memblock()
    {
        if( f_triggered ) return true;
        delete [] f_memblock;
        f_triggered = true;
        return true;
    }

}
