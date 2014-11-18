/////////////////////////////////////////////////
// Program to make egg files from roach1 board //
// Original Author: N.S.Oblath                 //
//		    nsoblath@mit.edu           //
// Modified by:     Prajwal Mohanmurthy        //
//                  prajwal@mohanmurthy.com    //
//		    MIT LNS                    //
//                  03/ 2014                   //
/////////////////////////////////////////////////
#include "mt_digitizer_roach_10gbe.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_katcp.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
#include <string>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_roach_10gbe" );

    MT_REGISTER_DIGITIZER( digitizer_roach_10gbe, "roach-10gbe" );


    const unsigned digitizer_roach_10gbe::s_data_type_size = sizeof( digitizer_roach_10gbe::data_type );

    unsigned digitizer_roach_10gbe::data_type_size_roach()
    {
        return digitizer_roach_10gbe::s_data_type_size;
    }

    digitizer_roach_10gbe::digitizer_roach_10gbe() :
            f_katcp_client(),
            f_bof_file(),
            //f_semaphore( NULL ),
            f_allocated( false ),
            f_buffer( NULL ),
            f_condition( NULL ),
            f_record_last( 0 ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_dead_time( 0 ),
            f_canceled( false ),
            f_cancel_condition(),
            fAcquireMode( request_mode_t_dual_interleaved )
    {
        get_calib_params( 8, s_data_type_size, -5.0, 10.0, &f_params );

        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_roach_10gbe", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_roach_10gbe is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
         */
    }

    digitizer_roach_10gbe::~digitizer_roach_10gbe()
    {
        if( f_allocated )
        {
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

    void digitizer_roach_10gbe::configure( const param_node* config )
    {
        f_katcp_client.set_server_ip( config->get_value( "roach-host" ) );
        f_katcp_client.set_timeout( config->get_value( "roach-timeout", f_katcp_client.get_timeout() ) );
        f_bof_file = config->get_value("roach-boffile", f_bof_file );
        return;
    }

    bool digitizer_roach_10gbe::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        if( ! f_katcp_client.connect() )
        {
            MTERROR( mtlog, "unable to connect to the ROACH board" );
            return false;
        }

        MTINFO( mtlog, "allocating buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                block* t_new_block = block::allocate_block< data_type >( f_buffer->block_size() );
                t_new_block->set_cleanup( new block_cleanup_roach_10gbe( t_new_block->data_bytes() ) );
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

    bool digitizer_roach_10gbe::initialize( request* a_request )
    {
        //MTINFO( mtlog, "resetting counters..." );
        
        if(a_request->mode() != request_mode_t_dual_interleaved)
        {
            fAcquireMode = a_request->mode(); //default to 'request_mode_t_dual_interleaved'
        }
        
        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->block_size()) ));
        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;
        f_dead_time = 0;

        if( borph_prog( f_bof_file ) < 0 )
        {
            MTERROR( mtlog,"Unable to program FPGA with bof file <" << f_bof_file << ">" );
            return false;
        }
        else
        {
            MTINFO( mtlog,"FPGA programmed with bof file <"<< f_bof_file << ">" );
        }

        if( borph_write( f_reg_name_ctrl, 0, 00  ) < 0 )
        {
            MTERROR( mtlog, "Unable to write to register - 'snap64_ctrl-00'" );
            return false;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl-00'");
        }

        if( borph_write( f_reg_name_ctrl, 0, 0111 ) < 0 )
        {
            MTERROR( mtlog,"Unable to write to register - 'snap64_ctrl-0111'" );
            return false;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl-0111'");
        }

        return true;
    }

    void digitizer_roach_10gbe::execute()
    {
        iterator t_it( f_buffer, "dig-roach" );

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
            return;
        }

        //start timing
        get_time_monotonic( &t_live_start_time );

        MTINFO( mtlog, "planning on " << f_record_last << " records" );

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

            //f_buffer->print_states();

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
                    // to make sure we don't deadlock anything
                    if( f_cancel_condition.is_waiting() )
                    {
                        f_cancel_condition.release();
                    }

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
            //MTINFO( mtlog, "record count: " << f_record_count );

        }

        return;
    }

    void digitizer_roach_10gbe::cancel()
    {
        //cout << "CANCELLING DIGITIZER TEST" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  digitizer_roach_10gbe is done canceling" );
        return;
    }

    void digitizer_roach_10gbe::finalize( response* a_response )
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

    bool digitizer_roach_10gbe::start()
    {
        return true;
    }

    bool digitizer_roach_10gbe::acquire( block* a_block, timespec& a_stamp_time )
    {
        //Katcp
        if( borph_read( f_reg_name_msb, a_block->data_bytes(), f_rm_half_record_size ) < 0 )
        {
            MTERROR( mtlog,"Unable to read register 'snap64_bram_msb'" );
            return false;
        }
        
/* ENABLE FOR SECOND CHANNEL
        if( borph_read( f_reg_name_lsb, a_block->data_bytes() + f_rm_half_record_size, f_rm_half_record_size ) < 0 )
        {
            MTERROR( mtlog,"Unable to read register 'snap64_bram_lsb'" );
            return false;
        }   
*/

        //End:Katcp

        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );
        ++f_record_count;
        return true;
    }

    bool digitizer_roach_10gbe::stop()
    {
        return true;
    }

    bool digitizer_roach_10gbe::write_mode_check( request_file_write_mode_t )
    {
        return true;
    }

    unsigned digitizer_roach_10gbe::data_type_size()
    {
        return digitizer_roach_10gbe::s_data_type_size;
    }

    bool digitizer_roach_10gbe::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_roach_10gbe::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }

    //********************************
    // Block Cleanup -- Test Digitizer
    //********************************

    block_cleanup_roach_10gbe::block_cleanup_roach_10gbe( digitizer_roach_10gbe::data_type* a_memblock ) :
            block_cleanup(),
            f_triggered( false ),
            f_memblock( a_memblock )
    {}
    block_cleanup_roach_10gbe::~block_cleanup_roach_10gbe()
    {}
    bool block_cleanup_roach_10gbe::delete_memblock()
    {
        if( f_triggered ) return true;
        delete [] f_memblock;
        f_triggered = true;
        return true;
    }

}
