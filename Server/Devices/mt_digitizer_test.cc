#define MANTIS_API_EXPORTS

#include "mt_digitizer_test.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"
#include "mt_param.hh"

#include "M3Constants.hh"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_test" );

    MT_REGISTER_DIGITIZER( digitizer_test, "test" );

    void digitizer_test_config_template::add( param_node* a_node, const std::string& a_type )
    {
        param_node* t_new_node = new param_node();
        t_new_node->add( "rate", param_value() << 250 );
        t_new_node->add( "n-channels", param_value() << 1 );
        t_new_node->add( "data-mode", param_value() << monarch3::sDigitizedUS );
        t_new_node->add( "channel-mode", param_value() << monarch3::sInterleaved );
        t_new_node->add( "sample-size", param_value() << 1 );
        t_new_node->add( "buffer-size", param_value() << 512 );
        t_new_node->add( "record-size", param_value() << 4194304 );
        t_new_node->add( "data-chunk-size", param_value() << 1024 );
        a_node->add( a_type, t_new_node );

    }

    const unsigned digitizer_test::s_data_type_size = sizeof( digitizer_test::data_type );
    unsigned digitizer_test::data_type_size_test()
    {
        return digitizer_test::s_data_type_size;
    }

    digitizer_test::digitizer_test() :
            //f_semaphore( NULL ),
            f_master_record( NULL ),
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
        get_calib_params( 8, s_data_type_size, -0.25, 0.5, &f_params );

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
        MTINFO( mtlog, "Allocating buffer" );

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
            MTERROR( mtlog, "Unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "Creating master record..." );

        if( f_master_record != NULL ) delete [] f_master_record;
        f_master_record = new data_type [f_buffer->block_size()];
        for( unsigned index = 0; index < f_buffer->block_size(); ++index )
        {
            f_master_record[ index ] = index % f_params.levels;
        }

        f_allocated = true;
        return true;
    }

    bool digitizer_test::deallocate()
    {
        delete [] f_master_record;

        MTINFO( mtlog, "Deallocating buffer" );

        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            f_buffer->delete_block( index );
        }
        f_allocated = false;
        return true;
     }

    bool digitizer_test::initialize( param_node* a_global_config, param_node* a_dev_config )
    {
        //MTINFO( mtlog, "resetting counters..." );

        a_dev_config->replace( "voltage-min", param_value() << f_params.v_min );
        a_dev_config->replace( "voltage-range", param_value() << f_params.v_range );
        a_dev_config->replace( "dac-gain", param_value() << f_params.dac_gain );

        // check buffer allocation
        // this section assumes 1 channel, in not multiplying t_actual_rec_size by the number of channels when converting to block size
        unsigned t_buffer_size = a_dev_config->get_value< unsigned >( "buffer-size", 512 );
        unsigned t_rec_size = a_dev_config->get_value< unsigned >( "record-size", 16384 );
        if( f_buffer != NULL && ( f_buffer->size() != t_buffer_size || f_buffer->block_size() != t_rec_size ) )
        {
            // need to redo the buffer
            if( f_allocated ) deallocate();
            delete f_buffer;
            f_buffer = NULL;
        }
        if( f_buffer == NULL )
        {
            f_buffer = new buffer( t_buffer_size, t_rec_size );
            allocate();
        }

        f_record_last = ( record_id_type )( ceil( ( double )( a_dev_config->get_value< double >( "rate" ) * a_global_config->node_at( "run" )->get_value< double >( "duration" ) * 1.e3 ) / ( double )( f_buffer->block_size() ) ) );
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        return true;
    }

    void digitizer_test::execute()
    {
        iterator t_it( f_buffer, "dig_test" );

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

            //MTDEBUG( mtlog, "digitizer_test:" );
            //f_buffer->print_states();

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

            // slow things down a bit, since this is for testing purposes, after all
#ifndef _WIN32
            usleep( 1000 );
#else
            Sleep(1);
#endif
        }

        return;
    }

    /* Asyncronous cancelation:
    Main execution loop checks for f_canceled, and exits if it's true.
    */
    void digitizer_test::cancel()
    {
        MTDEBUG(mtlog, "Canceling digitizer test");
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

    bool digitizer_test::start()
    {
        return true;
    }

    bool digitizer_test::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );

        ::memcpy( a_block->data_bytes(), f_master_record, f_buffer->block_size() );

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
        MTWARN( mtlog, "Basic test for digitizer_test has not been implemented" );
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
