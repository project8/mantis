#include "mt_digitizer_test16.hh"

#include "mt_bit_shift_modifier.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"
#include "mt_thread.hh"

#include "response.pb.h"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
//#include <fcntl.h> // for O_CREAT and O_EXCL

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_test16" );

    MT_REGISTER_DIGITIZER( digitizer_test16, "test16" );
    MT_REGISTER_TEST_DIGITIZER( test_digitizer_test16, "test16" );

    const unsigned digitizer_test16::s_data_type_size = 14; //sizeof( digitizer_test16::data_type );
    unsigned digitizer_test16::data_type_size_test()
    {
        return digitizer_test16::s_data_type_size;
    }

    digitizer_test16::digitizer_test16() :
            //f_semaphore( NULL ),
            f_master_record( NULL ),
            f_allocated( false ),
            f_buffer( NULL ),
            f_condition( NULL ),
            f_start_time( 0 ),
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_canceled( false ),
            f_cancel_condition()
    {
        get_calib_params( 14, s_data_type_size, -0.25, 0.5, &f_params );
        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_test16", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_test16 is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
        */
    }

    digitizer_test16::~digitizer_test16()
    {
        if( f_allocated )
        {
            delete [] f_master_record;

            MTINFO( mtlog, "deallocating buffer..." );

            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }
        }
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
        */
    }

    bool digitizer_test16::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        MTINFO( mtlog, "allocating buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                block* t_new_block = block::allocate_block< data_type >( f_buffer->record_size() );
                t_new_block->set_cleanup( new block_cleanup_test16( t_new_block->data_bytes() ) );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "creating master record..." );

        MTDEBUG( mtlog, "n levels: " << f_params.levels );
        if( f_master_record != NULL ) delete [] f_master_record;
        f_master_record = new data_type [f_buffer->record_size()];
        for( unsigned index = 0; index < f_buffer->record_size(); ++index )
        {
            f_master_record[ index ] = (index % f_params.levels) << 2;
            //if( index < 100 ) MTDEBUG( mtlog, "setting master record [" << index << "]: " << f_master_record[index] );
        }

        f_allocated = true;
        return true;
    }

    bool digitizer_test16::initialize( request* a_request )
    {
        //MTINFO( mtlog, "resetting counters..." );

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->record_size()) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        return true;
    }
    void digitizer_test16::execute()
    {
        iterator t_it( f_buffer, "dig_test16" );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;



        // bit shift modifier
        bit_shift_modifier< digitizer_test16::data_type > t_bs_mod;
        t_bs_mod.set_bit_shift( 2 );
        t_bs_mod.set_buffer( f_buffer, f_condition );
        thread t_bs_mod_thread( &t_bs_mod );
        t_bs_mod_thread.start();



        MTINFO( mtlog, "waiting for buffer readiness" );
        f_condition->wait();

        MTINFO( mtlog, "loose at <" << t_it.index() << ">" );

        // why was cancel disabled? -- Noah, 3/27/14
        //int t_old_cancel_state;
        //pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

        //start acquisition
        if( start() == false )
        {
            return;
        }

        MTINFO( mtlog, "planning on " << f_record_last << " records" );

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

                // remove the iterator from the buffer
                t_it.release();

                //halt the pci acquisition
                stop();

                //GET OUT
                if( f_canceled.load() )
                {
                    MTINFO( mtlog, "was canceled mid-run" );
                    f_cancel_condition.release();
                    MTDEBUG( mtlog, "canceling bs_mod thread" );
                    t_bs_mod_thread.cancel();
                }
                else
                {
                    MTINFO( mtlog, "finished normally" );
                    MTDEBUG( mtlog, "waiting for bs_mod thread to finish" );
                    t_bs_mod_thread.join();
                    MTDEBUG( mtlog, "bs_mod thread done" );
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

                // remove the iterator from the buffer
                t_it.release();

                //halt the pci acquisition
                stop();

                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                MTINFO( mtlog, "finished abnormally because acquisition failed" );

                MTDEBUG( mtlog, "canceling bs_mod thread" );
                t_bs_mod_thread.cancel();

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
                    MTDEBUG( mtlog, "canceling bs_mod thread" );
                    t_bs_mod_thread.cancel();
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
                    // to make sure we don't deadlock anything
                    if( f_cancel_condition.is_waiting() )
                    {
                        f_cancel_condition.release();
                    }

                    //GET OUT
                    MTINFO( mtlog, "finished abnormally because starting streaming failed" );

                    MTDEBUG( mtlog, "canceling bs_mod thread" );
                    t_bs_mod_thread.cancel();

                    return;
                }

                //increment block
                ++t_it;

                //start live timer
                get_time_monotonic( &t_live_start_time );;

                MTINFO( mtlog, "loose at <" << t_it.index() << ">" );
            }
            //MTINFO( mtlog, "record count: " << f_record_count );

            // slow things down a bit, since this is for testing purposes, after all
            usleep( 100 );
        }

        return;
    }
    void digitizer_test16::cancel()
    {
        //cout << "CANCELLING DIGITIZER TEST" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  digitizer_test16 is done canceling" );
        return;
    }
    void digitizer_test16::finalize( response* a_response )
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

    bool digitizer_test16::start()
    {
        return true;
    }
    bool digitizer_test16::acquire( block* a_block, timespec& a_stamp_time )
    {
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) - f_start_time );

        MTWARN( mtlog, "acquiring to: " << a_block->data_bytes() );
        ::memcpy( a_block->data_bytes(), (byte_type*)f_master_record, a_block->get_data_nbytes() );
        //for(unsigned index = 1000; index < 1050; ++index)
        //{
        //    MTERROR( mtlog, ((data_type*)(a_block->data_bytes()))[index]);
        //}

        ++f_record_count;

        return true;
    }
    bool digitizer_test16::stop()
    {
        ++f_acquisition_count;
        return true;
    }

    bool digitizer_test16::write_mode_check( request_file_write_mode_t )
    {
        return true;
    }

    unsigned digitizer_test16::data_type_size()
    {
        return digitizer_test16::s_data_type_size;
    }

    bool digitizer_test16::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_test16::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }


    //**********************************
    // Block Cleanup -- Test16 Digitizer
    //**********************************

    block_cleanup_test16::block_cleanup_test16( byte_type* a_data ) :
            block_cleanup(),
            f_triggered( false ),
            f_data( a_data )
    {}
    block_cleanup_test16::~block_cleanup_test16()
    {}
    bool block_cleanup_test16::delete_data()
    {
        if( f_triggered ) return true;
        delete [] f_data;
        f_triggered = true;
        return true;
    }
}
