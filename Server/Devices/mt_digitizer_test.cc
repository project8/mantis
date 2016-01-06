#define MANTIS_API_EXPORTS

#include "mt_digitizer_test.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"

#include "logger.hh"
#include "param.hh"

#include "M3Constants.hh"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL

using scarab::param_node;
using scarab::param_value;

namespace mantis
{
    LOGGER( mtlog, "digitizer_test" );

    MT_REGISTER_DIGITIZER( digitizer_test, "test" );

    void digitizer_test_config_template::add( param_node* a_node, const std::string& a_type )
    {
        param_node* t_new_node = new param_node();
        t_new_node->add( "rate", param_value( 250 ) );
        t_new_node->add( "data-mode", param_value( monarch3::sDigitizedUS ) );
        t_new_node->add( "channel-mode", param_value( monarch3::sSeparate ) );
        t_new_node->add( "sample-size", param_value( 1 ) );
        t_new_node->add( "buffer-size", param_value( 512 ) );
        t_new_node->add( "record-size", param_value( 4194304 ) );
        t_new_node->add( "data-chunk-size", param_value( 1024 ) );
        param_node* t_chan0_node = new param_node();
        t_chan0_node->add( "enabled", param_value( true ) );
        t_chan0_node->add( "voltage-range", param_value( 0.5 ) );
        t_chan0_node->add( "voltage-offset", param_value( 0.0 ) );
        param_node* t_chan1_node = new param_node();
        t_chan1_node->add( "enabled", param_value( true ) );
        t_chan1_node->add( "voltage-range", param_value( 0.5 ) );
        t_chan1_node->add( "voltage-offset", param_value( 0.0 ) );
        param_node* t_chan_node = new param_node();
        t_chan_node->add( "0", t_chan0_node );
        t_chan_node->add( "1", t_chan1_node );
        t_new_node->add( "channels", t_chan_node );
        a_node->add( a_type, t_new_node );

    }

    const unsigned digitizer_test::s_data_type_size = sizeof( digitizer_test::data_type );
    unsigned digitizer_test::data_type_size_test()
    {
        return digitizer_test::s_data_type_size;
    }

    const unsigned digitizer_test::s_n_channels = 2;
    const unsigned digitizer_test::s_bit_depth = 8;

    digitizer_test::digitizer_test() :
                    //f_semaphore( NULL ),
                    f_master_record( NULL ),
                    f_record_size( 0 ),
                    f_chan0_enabled( false ),
                    f_chan1_enabled( false ),
                    f_allocated( false ),
                    f_start_time( 0 ),
                    f_record_last( 0 ),
                    f_record_count( 0 ),
                    f_acquisition_count( 0 ),
                    f_live_time( 0 ),
                    f_dead_time( 0 ),
                    f_canceled( false ),
                    f_cancel_condition()
    {
        f_params = new dig_calib_params[ s_n_channels ];

        get_calib_params( 8, s_data_type_size, -0.25, 0.5, false, &( params( 0 ) ) );
        get_calib_params( 8, s_data_type_size, -0.5, 0.5, false, &( params( 1 ) ) );

        f_master_record = new data_type* [ s_n_channels ];
        f_master_record[ 0 ] = NULL;
        f_master_record[ 1 ] = NULL;

        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_test", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_test is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
         */
    }

    digitizer_test::~digitizer_test()
    {
        if( f_master_record != NULL )
        {
            if( f_master_record[ 0 ] != NULL ) delete [] f_master_record[ 0 ];
            if( f_master_record[ 1 ] != NULL ) delete [] f_master_record[ 1 ];
            delete [] f_master_record;
        }

        if( f_allocated ) deallocate();
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
         */
    }

    bool digitizer_test::allocate()
    {
        INFO( mtlog, "Allocating buffer" );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                block* t_new_block = block::allocate_block< data_type >( f_buffer->block_size() );
                t_new_block->set_cleanup( new block_cleanup_test( t_new_block->data_bytes() ) );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            ERROR( mtlog, "Unable to allocate buffer: " << e.what() );
            return false;
        }

        f_allocated = true;
        return true;
    }

    bool digitizer_test::deallocate()
    {
        INFO( mtlog, "Deallocating buffer" );

        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            f_buffer->delete_block( index );
        }
        f_allocated = false;
        return true;
    }

    void digitizer_test::allocate_master_records( size_t a_rec_size, bool a_allocate_chan_0, bool a_allocate_chan_1 )
    {
        INFO( mtlog, "Creating master records..." );

        f_record_size = a_rec_size;

        if( f_master_record[ 0 ] != NULL ) delete [] f_master_record[ 0 ];
        if( f_master_record[ 1 ] != NULL ) delete [] f_master_record[ 1 ];

        if( a_allocate_chan_0 )
        {
            f_master_record[ 0 ] = new data_type[ f_record_size ];
            std::cout << "channel 0: ";
            for( unsigned index = 0; index < f_record_size; ++index )
            {
                f_master_record[ 0 ][ index ] = index % f_params[ 0 ].levels;
                if( index < 100 ) std::cout << (unsigned)f_master_record[ 0 ][ index ] << ", ";
            }
            std::cout << std::endl;
        }

        if( a_allocate_chan_1 )
        {
            f_master_record[ 1 ] = new data_type[ f_record_size ];
            data_type t_value = f_params[ 1 ].levels / 2;
            std::cout << "channel 1: ";
            for( unsigned index = 0; index < f_record_size; ++index )
            {
                f_master_record[ 1 ][ index ] = t_value;
                if( index < 100 ) std::cout << (unsigned)f_master_record[ 1 ][ index ] << ", ";
            }
            std::cout << std::endl;
        }

        return;
    }

    bool digitizer_test::initialize( param_node* a_global_config, param_node* a_dev_config )
    {
        //INFO( mtlog, "resetting counters..." );


        param_node* t_channels_config = a_dev_config->node_at( "channels" );
        if( t_channels_config == NULL )
        {
            ERROR( mtlog, "Did not find a \"channels\" node" );
            return false;
        }

        param_node* t_chan_config[ 2 ] = {t_channels_config->node_at( "0" ), t_channels_config->node_at( "1" )};
        if( t_chan_config[ 0 ] == NULL || t_chan_config[ 1 ] == NULL )
        {
            ERROR( mtlog, "Invalid device config: unable to find configuration for either channel 0 (" << t_chan_config[ 0 ] << ") or channel 1 (" << t_chan_config[ 1 ] << ")" );
            return false;
        }

        // check which channels are enabled
        unsigned n_chan_enabled = 0;

        f_chan0_enabled = false;
        if( t_chan_config[ 0 ]->get_value< bool >( "enabled", false ) )
        {
            ++n_chan_enabled;
            f_chan0_enabled = true;
        }

        f_chan1_enabled = false;
        if( t_chan_config[ 1 ]->get_value< bool >( "enabled", false ) )
        {
            ++n_chan_enabled;
            f_chan1_enabled = true;
        }

        if( n_chan_enabled == 0 )
        {
            ERROR( mtlog, "No channels were enabled" );
            return false;
        }
        a_dev_config->replace( "n-channels", new param_value( n_chan_enabled ) );

        DEBUG( mtlog, "Recording from " << n_chan_enabled << " channel(s): " << f_chan0_enabled << ", " << f_chan1_enabled );

        // Check data mode and channel mode
        uint32_t t_data_mode = a_dev_config->get_value< uint32_t >( "data-mode" );
        if( t_data_mode != monarch3::sDigitizedUS )
        {
            ERROR( mtlog, "Data can only be taken in <digitized-unsigned> mode" );
            return false;
        }
        if( a_dev_config->get_value< uint32_t >( "channel-mode" ) != monarch3::sSeparate )
        {
            ERROR( mtlog, "Multi-channel data can only be recorded in <separate> mode" );
            return false;
        }

        // check buffer and master record allocations
        bool t_must_allocate = false; // will be done later, assuming the initialization succeeds
        unsigned t_buffer_size = a_dev_config->get_value< unsigned >( "buffer-size", 512 );
        unsigned t_record_size = a_dev_config->get_value< unsigned >( "record-size", 16384 );
        unsigned t_block_size_needed = t_record_size * n_chan_enabled;

        if( t_record_size != f_record_size ||
                ( f_chan0_enabled && f_master_record[ 0 ] == NULL ) ||
                ( f_chan1_enabled && f_master_record[ 1 ] == NULL ) )
        {
            allocate_master_records( t_record_size, f_chan0_enabled, f_chan1_enabled );
        }

        if( f_buffer != NULL && ( f_buffer->size() != t_buffer_size || f_buffer->block_size() != t_block_size_needed ) )
        {
            // need to redo the buffer
            if( f_allocated ) deallocate();
            delete f_buffer;
            f_buffer = NULL;
        }
        if( f_buffer == NULL )
        {
            t_must_allocate = true;
            f_buffer = new buffer( t_buffer_size, t_block_size_needed );
        }

        // configure channel-dependent features
        for( unsigned i_chan = 0; i_chan < 1; ++i_chan )
        {
            std::stringstream t_conv;
            t_conv << i_chan;
            std::string t_this_chan_string( t_conv.str() );

            // call to niScope_ConfigureVertical
            double t_voltage_range = t_chan_config[ i_chan ]->get_value< double >( "voltage-range", 0.5 );
            double t_voltage_offset = t_chan_config[ i_chan ]->get_value< double >( "voltage-offset", 0. );
            get_calib_params( s_bit_depth, s_data_type_size, t_voltage_offset, t_voltage_range, false, &( f_params[i_chan] ) );
            t_chan_config[ i_chan ]->replace( "dac-gain", param_value( f_params[ i_chan ].dac_gain ) );
        }

        // allocate the buffer if needed
        if( t_must_allocate )
        {
            allocate();
        }

        f_record_last = ( record_id_type )( ceil( ( double )( a_dev_config->get_value< double >( "rate" ) * a_global_config->get_value< double >( "duration" ) * 1.e3 ) / ( double )( f_record_size ) ) );
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        return true;
    }

    void digitizer_test::execute()
    {
        if( f_status != k_ok )
        {
            ERROR( mtlog, "Digitizer status is not \"ok\"" );
            return;
        }

        iterator t_it( f_buffer, "dig_test" );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        //INFO( mtlog, "waiting" );

        f_buffer_condition->wait();

        INFO( mtlog, "Digitizer loose at <" << t_it.index() << ">" );

        // nsoblath, 1/30/15: why did i have this here before?
        //int t_old_cancel_state;
        //pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

        //start acquisition
        if( start() == false )
        {
            set_status( k_error, "Unable to start acquisition" );
            return;
        }

        INFO( mtlog, "Planning on " << f_record_last << " records" );

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
                    INFO( mtlog, "Digitizer was canceled mid-run" );
                    set_status( k_error, "Digitizer was canceled mid-run" );
                    f_cancel_condition.release();
                }
                else
                {
                    set_status( k_ok, "Finished normally" );
                    INFO( mtlog, "Finished normally" );
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
                set_status( k_error, "Finished abnormally because acquisition failed" );
                INFO( mtlog, "Finished abnormally because acquisition failed" );

                return;
            }

            //DEBUG( mtlog, "digitizer_test:" );
            //f_buffer->print_states();

            t_it->set_acquired();

            if( +t_it == false )
            {
                INFO( mtlog, "blocked at <" << t_it.index() << ">" );

                //stop live timer
                get_time_monotonic( &t_live_stop_time );

                //accumulate live time
                f_live_time += time_to_nsec( t_live_stop_time ) - time_to_nsec( t_live_start_time );

                //halt the pci acquisition
                if( stop() == false )
                {
                    //GET OUT
                    set_status( k_error, "Finished abnormally because halting streaming failed" );
                    INFO( mtlog, "Finished abnormally because halting streaming failed" );
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
                    set_status( k_error, "Finished abnormally because starting streaming failed" );
                    INFO( mtlog, "Finished abnormally because starting streaming failed" );
                    return;
                }

                //increment block (waits for mutex lock)
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                INFO( mtlog, "Loose at <" << t_it.index() << ">" );
            }
            //INFO( mtlog, "record count: " << f_record_count );

            // slow things down a bit, since this is for testing purposes, after all
#ifndef _WIN32
            usleep( 1000 );
#else
            Sleep(1);
#endif
        }

        set_status( k_error, "This section of code should not be reached" );
        return;
    }

    /* Asyncronous cancelation:
    Main execution loop checks for f_canceled, and exits if it's true.
     */
    void digitizer_test::cancel()
    {
        DEBUG(mtlog, "Canceling digitizer test");
        //cout << "CANCELLING DIGITIZER TEST" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  digitizer_test is done canceling" );
        return;
    }

    void digitizer_test::finalize( param_node* a_response )
    {
        //INFO( mtlog, "calculating statistics..." );
        double t_livetime = (double) (f_live_time) * SEC_PER_NSEC;
        double t_deadtime = (double) f_dead_time * SEC_PER_NSEC;
        double t_mb_recorded = (double) (4 * f_record_count);

        param_node* t_resp_node = new param_node();
        t_resp_node->add( "record-count", param_value( f_record_count ) );
        t_resp_node->add( "acquisition-count", param_value( f_acquisition_count ) );
        t_resp_node->add( "livetime", param_value( t_livetime ) );
        t_resp_node->add( "deadtime", param_value( t_deadtime ) );
        t_resp_node->add( "mb-recorded", param_value( t_mb_recorded ) );
        t_resp_node->add( "digitization-rate", param_value( t_mb_recorded / t_livetime ) );

        a_response->add( "digitizer-test", t_resp_node );

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

        // all of this assumes the data type size is 1, so f_record_size is the length of the record in bytes as well as samples
        unsigned t_write_offset = 0;
        // copy first channel
        if( f_chan0_enabled )
        {
            ::memcpy( a_block->data_bytes(), f_master_record[ 0 ], f_record_size );
            t_write_offset += f_record_size;
        }
        // copy second channel
        if( f_chan1_enabled )
        {
            ::memcpy( a_block->data_bytes() + t_write_offset, f_master_record[ 1 ], f_record_size );
        }

        // the timestamp is acquired after the data is transferred to avoid the problem on the px1500 where
        // the first record can take unusually long to be acquired.
        // it's done here too for consistency
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) - f_start_time );

        ++f_record_count;

        return true;
    }

    bool digitizer_test::stop()
    {
        ++f_acquisition_count;
        return true;
    }

    unsigned digitizer_test::data_type_size()
    {
        return digitizer_test::s_data_type_size;
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

    bool digitizer_test::run_basic_test()
    {
        WARN( mtlog, "Basic test for digitizer_test has not been implemented" );
        return false;
    }


    //********************************
    // Block Cleanup -- Test Digitizer
    //********************************

    block_cleanup_test::block_cleanup_test( byte_type* a_memblock ) :
                    block_cleanup(),
                    f_triggered( false ),
                    f_memblock( a_memblock )
    {}

    block_cleanup_test::~block_cleanup_test()
    {}

    bool block_cleanup_test::delete_memblock()
    {
        if( f_triggered ) return true;
        delete [] f_memblock;
        f_triggered = true;
        return true;
    }

}
