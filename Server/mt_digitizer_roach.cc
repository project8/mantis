/////////////////////////////////////////////////
// Program to make egg files from roach1 board //
// Original Author: N.S.Oblath                 //
//		            nsoblath@mit.edu           //
// Modified by:     Prajwal Mohanmurthy        //
//                  prajwal@mohanmurthy.com    //
//		            MIT LNS                    //
//                  03/ 2014                   //
/////////////////////////////////////////////////
#include "mt_digitizer_roach.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_iterator.hh"
#include "mt_logger.hh"

#include "response.pb.h"

#include <cmath> // for ceil()
#include <cstdlib> // for exit()
#include <cstring> // for memset()
#include <errno.h>
#include <string>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_roach" );

    MT_REGISTER_DIGITIZER( digitizer_roach, "roach" );


    const unsigned digitizer_roach::s_data_type_size = sizeof( digitizer_roach::data_type );

    unsigned digitizer_roach::data_type_size_roach()
    {
        return digitizer_roach::s_data_type_size;
    }

    digitizer_roach::digitizer_roach() :
            f_katcp_server(),
            f_katcp_cmdline( NULL ),
            f_katcp_fd( 0 ),
            f_rm_half_record_size( 0 ),
            f_rm_timeout( 5000 ),
            f_bof_file(),
            f_datax0( NULL ),
            f_datax1( NULL ),
            f_reg_name_msb( "snap64_bram_msb" ),
            f_reg_name_lsb( "snap64_bram_lsb" ),
            f_reg_name_ctrl( "snap64_ctrl" ),
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
            f_cancel_condition()
    {
        get_calib_params( 8, s_data_type_size, -5.0, 10.0, &f_params );

        strcpy( f_write_start,   "?write" );
        strcpy( f_write_end,     "!write" );
        strcpy( f_prog_start,    "?progdev" );
        strcpy( f_prog_end,      "!progdev" );
        strcpy( f_read_start,    "?read" );
        strcpy( f_read_end,      "!read" );

        /*
        errno = 0;
        f_semaphore = sem_open( "/digitizer_roach", O_CREAT | O_EXCL );
        if( f_semaphore == SEM_FAILED )
        {
            if( errno == EEXIST )
            {
                throw exception() << "digitizer_roach is already in use";
            }
            else
            {
                throw exception() << "semaphore error: " << strerror( errno );
            }
        }
         */
    }

    digitizer_roach::~digitizer_roach()
    {
        if( f_allocated )
        {
            MTINFO( mtlog, "deallocating buffer..." );

            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }

            delete [] f_datax0;
            delete [] f_datax1;
        }

        destroy_katcl(f_katcp_cmdline, 1);

        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
         */
    }

    void digitizer_roach::configure( const param_node* config )
    {
        f_katcp_server = config->get_value("roach-host");
        f_bof_file = config->get_value("roach-boffile");
        return;
    }

    //From Katcp Stuff:
    int digitizer_roach::dispatch_client( char *msgname, int verbose )
    {
        fd_set fsr, fsw;
        struct timeval tv;
        int result;
        char *ptr, *match;
        int prefix;

        f_katcp_fd = fileno_katcl( f_katcp_cmdline );

        if( msgname )
        {
            switch( msgname[0] )
            {
                case '!':
                case '?':
                    prefix = strlen(msgname + 1);
                    match = msgname + 1;
                    break;
                default:
                    prefix = strlen(msgname);
                    match = msgname;
                    break;
            }
        } 
        else
        {
            prefix = 0;
            match = NULL;
        }

        for(;;)
        {

            FD_ZERO( &fsr );
            FD_ZERO( &fsw );

            if( match )
            { /* only look for data if we need it */
                FD_SET( f_katcp_fd, &fsr );
            }

            if( flushing_katcl( f_katcp_cmdline ) )
            { /* only write data if we have some */
                FD_SET( f_katcp_fd, &fsw );
            }

            tv.tv_sec  = f_rm_timeout / 1000;
            tv.tv_usec = ( f_rm_timeout % 1000 ) * 1000;

            result = select( f_katcp_fd + 1, &fsr, &fsw, NULL, &tv );
            switch(result)
            {
                case -1 :
                    switch (errno )
                    {
                        case EAGAIN :
                        case EINTR  :
                            continue; /* WARNING */
                        default     :
                            return -1;
                    }
                    break;
                case  0 :
                    if(verbose)
                    {
                        MTERROR( mtlog, "dispatch: no io activity within " << f_rm_timeout << " ms" );
                    }
                    return -1;
            }

            if( FD_ISSET( f_katcp_fd, &fsw ) )
            {
                result = write_katcl( f_katcp_cmdline );
                if( result < 0 )
                {
                    MTERROR( mtlog, "dispatch: write failed:" << strerror( error_katcl( f_katcp_cmdline ) ) );
                    return -1;
                }
                if( ( result > 0 ) && ( match == NULL ) )
                { /* if we finished writing and don't expect a match then quit */
                    return 0;
                }
            }

            if( FD_ISSET( f_katcp_fd, &fsr ) )
            {
                result = read_katcl( f_katcp_cmdline );
                if( result )
                {
                    MTERROR( mtlog, "dispatch: read failed : " << strerror( error_katcl( f_katcp_cmdline) ) << " : connection terminated" );
                    return -1;
                }
            }

            while( have_katcl( f_katcp_cmdline ) > 0)
            {
                ptr = arg_string_katcl( f_katcp_cmdline, 0 );
                if( ptr )
                {
#ifdef DEBUG
                    MTERROR( mtlog, "dispatch: got back " << ptr );
#endif
                    switch( ptr[0] )
                    {
                        case KATCP_INFORM :
                            break;
                        case KATCP_REPLY  :
                            if(match)
                            {
                                if(strncmp( match, ptr + 1, prefix ) ||
                                        ( ( ptr[prefix + 1] != '\0' ) && ( ptr[prefix + 1] != ' ') ) )
                                {
                                    MTERROR( mtlog, "dispatch: warning, encountered reply " << ptr << " not match " << match );
                                }
                                else
                                {
                                    ptr = arg_string_katcl( f_katcp_cmdline, 1 );
                                    if( ptr && ! strcmp( ptr, KATCP_OK ) )
                                    {
                                        return 0;
                                    }
                                    else
                                    {
                                        return -1;
                                    }
                                }
                            }
                            break;
                        case KATCP_REQUEST :
                            MTERROR( mtlog, "dispatch: warning, encountered an unanswerable request " << ptr );
                            break;
                        default :
                            MTERROR(mtlog,"dispatch: read malformed message "<<ptr);
                            break;
                    }
                }
            }
        }
        return 0;
    }
    /////

    int digitizer_roach::borph_write( const string& a_regname, int buffer, int len )
    {
        /* populate a request */
        if( append_string_katcl( f_katcp_cmdline, KATCP_FLAG_FIRST, f_write_start ) < 0)
            return -1;
        if( append_string_katcl( f_katcp_cmdline, 0, f_write_end ) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_katcp_cmdline, 0, 0) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_katcp_cmdline, 0, buffer) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_katcp_cmdline, KATCP_FLAG_LAST, len) < 0 )
            return -1;

        /* use above function to send request */
        if( dispatch_client( f_write_end , 1 ) < 0 )
            return -1;

        /* clean up request for next call */
        have_katcl( f_katcp_cmdline );

        return 0;
    }

    ///////

    int digitizer_roach::borph_prog( const string& a_bof_file )
    {   
        /* populate a request */
        if( append_string_katcl( f_katcp_cmdline, KATCP_FLAG_FIRST, f_prog_start ) < 0 )
            return -1;
        if( append_string_katcl( f_katcp_cmdline, KATCP_FLAG_LAST, const_cast< char* >( a_bof_file.c_str() ) ) < 0 )
            return -1;

        /* use above function to send request */
        if( dispatch_client( f_prog_end, 1 ) < 0 )
            return -1;

        /* clean up request for next call */
        have_katcl( f_katcp_cmdline );

        return 0;
    }

    ///////

    int digitizer_roach::borph_read( const string& a_regname, void* a_buffer, int a_len )
    {
        if( append_string_katcl( f_katcp_cmdline, KATCP_FLAG_FIRST, f_read_start ) < 0 )
            return -1;
        if( append_string_katcl( f_katcp_cmdline, 0, const_cast< char* >( a_regname.c_str() ) ) < 0 )
            return -1;
        if( append_unsigned_long_katcl( f_katcp_cmdline, 0, 0 ) < 0 )
            return -1;
        if( append_unsigned_long_katcl( f_katcp_cmdline, KATCP_FLAG_LAST, a_len ) < 0 )
            return -1;

        if( dispatch_client( f_read_end, 1 ) < 0 )
            return -1;

        int t_count = arg_count_katcl( f_katcp_cmdline );
        if( t_count < 2 )
        {
            MTERROR( mtlog,"insufficient arguments in reply" );
            return -1;
        }

        int t_got = arg_buffer_katcl( f_katcp_cmdline, 2, a_buffer, a_len );
        if( t_got < a_len )
        {
            MTERROR( mtlog,"partial data, wanted "<< a_len <<", got "<< t_got );
            return -1;
        }

        have_katcl( f_katcp_cmdline );

        return a_len;
    }

    //////End:Katcp Desc.//////////

    bool digitizer_roach::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;

        if( f_buffer->record_size() > 65536*4*2 )
        {
            MTERROR( mtlog, "Record size must be <= 65536*4*2 = 524288" );
            return false;   
        }

        MTINFO( mtlog, "connecting to katcp server..." );

        // Connect to the ROACH board
        if( f_katcp_server.empty() )
        {
            MTERROR( mtlog,"Please provide the host address for the ROACH system" );
            return false;
        }

        // get the file descriptor
        f_katcp_fd = net_connect( const_cast< char* >( f_katcp_server.c_str() ), 0, NETC_VERBOSE_ERRORS | NETC_VERBOSE_STATS );
        if( f_katcp_fd < 0 )
        {
            MTINFO( mtlog,"Unable to connect to the ROACH board at <"<< f_katcp_server << ">" );
            return false;
        }
        else
        {
            MTINFO( mtlog,"Connected to ROACH board at <" << f_katcp_server << ">" );
        }

        // katcp command line
        destroy_katcl( f_katcp_cmdline, 0 );
        f_katcp_cmdline = create_katcl( f_katcp_fd );
        if( f_katcp_cmdline == NULL )
        {
            MTERROR( mtlog, "Unable to allocate katcp command line" );
            return false;
        }

        MTINFO( mtlog, "allocating buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                block* t_new_block = block::allocate_block< data_type >( f_buffer->record_size() );
                t_new_block->set_cleanup( new block_cleanup_roach( t_new_block->data_bytes() ) );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "allocating katcp memory..." );

        f_rm_half_record_size = f_buffer->record_size() / 2;
        f_datax0 = new data_type [f_rm_half_record_size];
        f_datax1 = new data_type [f_rm_half_record_size];

        f_allocated = true;
        return true;
    }

    bool digitizer_roach::initialize( request* a_request )
    {
        //MTINFO( mtlog, "resetting counters..." );

        f_record_last = (record_id_type) (ceil( (double) (a_request->rate() * a_request->duration() * 1.e3) / (double) (f_buffer->record_size()) ));
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
            MTERROR( mtlog, "Unable to write to register - 'snap64_ctrl'" );
            return false;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl'");
        }

        if( borph_write( f_reg_name_ctrl, 0, 0111 ) < 0 )
        {
            MTERROR( mtlog,"Unable to write to register - 'snap64_ctrl'" );
            return false;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl'");
        }

        return true;
    }

    void digitizer_roach::execute()
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

    void digitizer_roach::cancel()
    {
        //cout << "CANCELLING DIGITIZER TEST" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  digitizer_roach is done canceling" );
        return;
    }

    void digitizer_roach::finalize( response* a_response )
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

    bool digitizer_roach::start()
    {
        return true;
    }

    bool digitizer_roach::acquire( block* a_block, timespec& a_stamp_time )
    {
        //Katcp
        if( borph_read( f_reg_name_msb, f_datax0, f_rm_half_record_size ) < 0 )
        {
            MTERROR( mtlog,"Unable to read register 'snap64_bram_msb'" );
            return false;
        }
        //else
        //{
        //    MTINFO(mtlog,"Read - 'snap64_bram_msb'");
        //}
        //printf("%d \n",(uint8_t)(f_datax0[1]));

        if( borph_read( f_reg_name_lsb, f_datax1, f_rm_half_record_size ) < 0 )
        {
            MTERROR( mtlog,"Unable to read register 'snap64_bram_lsb'" );
            return false;
        }   
        //else
        //{
        //    MTINFO(mtlog,"Read - 'snap64_bram_lsb'");
        //}

        // merge datax and datax1 into datay
        for( unsigned rm_index = 0; rm_index < f_rm_half_record_size; rm_index += 2 )
        {
            a_block->data_bytes()[ rm_index*2     ] = f_datax0[ rm_index ];
            a_block->data_bytes()[ rm_index*2 + 1 ] = f_datax1[ rm_index ];
        }
        //End:Katcp

        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );
        ++f_record_count;
        return true;
    }

    bool digitizer_roach::stop()
    {
        return true;
    }

    bool digitizer_roach::write_mode_check( request_file_write_mode_t )
    {
        return true;
    }

    unsigned digitizer_roach::data_type_size()
    {
        return digitizer_roach::s_data_type_size;
    }

    bool digitizer_roach::get_canceled()
    {
        return f_canceled.load();
    }

    void digitizer_roach::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }

    //********************************
    // Block Cleanup -- Test Digitizer
    //********************************

    block_cleanup_roach::block_cleanup_roach( digitizer_roach::data_type* a_memblock ) :
            block_cleanup(),
            f_triggered( false ),
            f_memblock( a_memblock )
    {}
    block_cleanup_roach::~block_cleanup_roach()
    {}
    bool block_cleanup_roach::delete_memblock()
    {
        if( f_triggered ) return true;
        delete [] f_memblock;
        f_triggered = true;
        return true;
    }

}