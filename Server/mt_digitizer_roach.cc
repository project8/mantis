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

namespace mantis
{
    MTLOGGER( mtlog, "digitizer_roach" );
    
    MT_REGISTER_DIGITIZER( digitizer_roach, "roach" );
    MT_REGISTER_TEST_DIGITIZER( test_digitizer_roach, "roach" );


    const unsigned digitizer_roach::s_data_type_size = sizeof( digitizer_roach::data_type );
    
    /*//katcp
    const int digitizer_roach::f_RM_recordSize = 65536*4;
    unsigned char digitizer_roach::f_datax[f_RM_recordSize];
	unsigned char digitizer_roach::f_datax1[f_RM_recordSize];
	uint8_t digitizer_roach::f_datay[f_RM_recordSize*2];*/
    
    
    unsigned digitizer_roach::data_type_size_roach()
    {
        return digitizer_roach::s_data_type_size;
    }

    digitizer_roach::digitizer_roach() :
            //f_semaphore( NULL ),
            f_master_record( NULL ),
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
            f_roach_ipaddress( "" ),
            f_roach_boffile( "" ),
            f_katcp_fd( 0 )
    {
        get_calib_params( 8, s_data_type_size, -0.25, 0.5, &f_params );
        f_RM_timeout = 5000;
        //boffile = const_cast<char*>("adc.bof");
        //f_katcp_server = getenv("KATCP_SERVER");
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
            delete [] f_master_record;

            MTINFO( mtlog, "deallocating buffer..." );

            for( unsigned int index = 0; index < f_buffer->size(); index++ )
            {
                f_buffer->delete_block( index );
            }
        }
        //destroy_katcl(f_katcp_cmdline, 1);
        /*
        if( f_semaphore != SEM_FAILED )
        {
            sem_close( f_semaphore );
        }
        */
    }

    void digitizer_roach::configure( const param_node* config )
    {
        f_roach_ipaddress = config->get_value("roach-host");
        f_roach_boffile = config->get_value("roach-boffile");
    }
        
    //From Katcp Stuff:
    int digitizer_roach::dispatch_client(struct katcl_line *f_katcp_cmdline, char *msgname, int verbose, unsigned int timeout)
    {
        fd_set fsr, fsw;
        struct timeval tv;
        int result;
        char *ptr, *match;
        int prefix;
        int f_katcp_fd;

        f_katcp_fd = fileno_katcl( f_katcp_cmdline );

        if(msgname)
        {
            switch(msgname[0])
            {
                case '!' :
                case '?' : prefix = strlen(msgname + 1);
                           match = msgname + 1;
                           break;
                default  : prefix = strlen(msgname);
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

            FD_ZERO(&fsr);
            FD_ZERO(&fsw);

            if(match)
            { /* only look for data if we need it */
                FD_SET(f_katcp_fd, &fsr);
            }

            if(flushing_katcl(f_katcp_cmdline))
            { /* only write data if we have some */
                FD_SET(f_katcp_fd, &fsw);
            }

            tv.tv_sec  = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;

            result = select(f_katcp_fd + 1, &fsr, &fsw, NULL, &tv);
            switch(result)
            {
                case -1 : switch(errno)
                          {
                                case EAGAIN :
                                case EINTR  : continue; /* WARNING */
                                default     : return -1;
                          }
                          break;
                case  0 : if(verbose)
                          {
                                MTERROR(mtlog,"dispatch: no io activity within "<<timeout<<" ms");
                          }
                          return -1;
            }

            if(FD_ISSET(f_katcp_fd, &fsw))
            {
                result = write_katcl(f_katcp_cmdline);
                if(result < 0)
                {
      	            MTERROR(mtlog,"dispatch: write failed:"<<strerror(error_katcl(f_katcp_cmdline)));
      	            return -1;
                }
                if((result > 0) && (match == NULL))
                { /* if we finished writing and don't expect a match then quit */
      	            return 0;
                }
            }

            if(FD_ISSET(f_katcp_fd, &fsr))
            {
                result = read_katcl(f_katcp_cmdline);
                if(result)
                {
      	            MTERROR(mtlog,"dispatch: read failed : "<<strerror(error_katcl(f_katcp_cmdline))<<" : connection terminated");
      	            return -1;
                }
            }

            while(have_katcl(f_katcp_cmdline) > 0)
            {
                ptr = arg_string_katcl(f_katcp_cmdline, 0);
                if(ptr)
                {
                    #ifdef DEBUG
                    MTERROR(mtlog,"dispatch: got back "<<ptr);
                    #endif
      	            switch(ptr[0])
      	            {
      	                case KATCP_INFORM : break;
                        case KATCP_REPLY  : if(match)
                                            {
                                                if(strncmp(match, ptr + 1, prefix) || ((ptr[prefix + 1] != '\0') && (ptr[prefix + 1] != ' ')))
                                                {
                                                    MTERROR(mtlog,"dispatch: warning, encountered reply "<<ptr<<" not match "<<match);
                                                } 
                                                else
                                                {
                                                    ptr = arg_string_katcl(f_katcp_cmdline, 1);
                                                    if(ptr && !strcmp(ptr, KATCP_OK))
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
                        case KATCP_REQUEST : MTERROR(mtlog,"dispatch: warning, encountered an unanswerable request "<<ptr);
                                             break;
                        default : MTERROR(mtlog,"dispatch: read malformed message "<<ptr);
                                  break;
                    }
                }
            }
        }
    }
    /////
    
    int digitizer_roach::borph_write(struct katcl_line *f_katcp_cmdline, char *regname, int buffer, int len, unsigned int timeout)
    {
        /* populate a request */
        if(append_string_katcl(f_katcp_cmdline, KATCP_FLAG_FIRST, const_cast<char*>("?write"))   < 0) return -1;
        if(append_string_katcl(f_katcp_cmdline, 0, regname)                   < 0) return -1;
        if(append_unsigned_long_katcl(f_katcp_cmdline, 0, 0)                  < 0) return -1;
        if(append_unsigned_long_katcl(f_katcp_cmdline, 0, buffer)             < 0) return -1;
        if(append_unsigned_long_katcl(f_katcp_cmdline, KATCP_FLAG_LAST, len)         < 0) return -1;

        /* use above function to send request */
        if(dispatch_client(f_katcp_cmdline, const_cast<char*>("!write"), 1, timeout)             < 0) return -1;

        /* clean up request for next call */
        have_katcl(f_katcp_cmdline);

        return 0;
    }
    
    ///////
    
    int digitizer_roach::borph_prog(struct katcl_line *f_katcp_cmdline, char *f_boffile, unsigned int timeout)
    {   
        /* populate a request */
        if(append_string_katcl(f_katcp_cmdline, KATCP_FLAG_FIRST, const_cast<char*>("?progdev")) < 0) return -1;
        if(append_string_katcl(f_katcp_cmdline, KATCP_FLAG_LAST, f_boffile)     < 0) return -1;
  
        /* use above function to send request */
        if(dispatch_client(f_katcp_cmdline, const_cast<char*>("!progdev"), 1, timeout)             < 0) return -1;

        /* clean up request for next call */
        have_katcl(f_katcp_cmdline);

        return 0;
    }

    ///////
    
    int digitizer_roach::borph_read(struct katcl_line *f_katcp_cmdline, char *regname, void *buffer, int len, unsigned int timeout)
    {
        int count, got;

        if(append_string_katcl(f_katcp_cmdline, KATCP_FLAG_FIRST, const_cast<char*>("?read"))    < 0) return -1;
        if(append_string_katcl(f_katcp_cmdline, 0, regname)                   < 0) return -1;
        if(append_unsigned_long_katcl(f_katcp_cmdline, 0, 0)                  < 0) return -1;
        if(append_unsigned_long_katcl(f_katcp_cmdline, KATCP_FLAG_LAST, len)  < 0) return -1;

        if(dispatch_client(f_katcp_cmdline, const_cast<char*>("!read"), 1, timeout)              < 0) return -1;

        count = arg_count_katcl(f_katcp_cmdline);
        if(count < 2)
        {
            MTERROR(mtlog,"read: insufficient arguments in reply");
            return -1;
        }
        got = arg_buffer_katcl(f_katcp_cmdline, 2, buffer, len);

        if(got < len)
        {
            MTERROR(mtlog,"read: partial data, wanted "<<len<<", got "<< got);
            return -1;
        }

        have_katcl(f_katcp_cmdline);

        return len;
    }
    
    //////End:Katcp Desc.//////////
    
    bool digitizer_roach::allocate( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;
        
        //katcp
        f_katcp_server = const_cast<char*>(f_roach_ipaddress.c_str());
        f_boffile = const_cast<char*>(f_roach_boffile.c_str());
        f_katcp_cmdline = create_katcl(f_katcp_fd);
        
        //end:katcp

        MTINFO( mtlog, "allocating buffer..." );

        try
        {
            for( unsigned int index = 0; index < f_buffer->size(); ++index )
            {
                typed_block< data_type >* t_new_block = new typed_block< data_type >();
                *( t_new_block->handle() ) = new data_type [ f_buffer->record_size() ];
                t_new_block->set_data_size( f_buffer->record_size() );
                t_new_block->set_cleanup( new block_cleanup_roach( t_new_block->data() ) );
                f_buffer->set_block( index, t_new_block );
            }
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to allocate buffer: " << e.what() );
            return false;
        }

        MTINFO( mtlog, "creating master record..." );

/*
        if( f_master_record != NULL ) delete [] f_master_record;
        f_master_record = new data_type [f_buffer->record_size()];
        for( unsigned index = 0; index < f_buffer->record_size(); ++index )
        {
            f_master_record[ index ] = index % f_params.levels;
        }
*/
        //for Katcp
        if(f_buffer->record_size() > 65536*4)
        {
            MTERROR( mtlog, "Record size must be < 65536*4" );
            return false;   
        }
        f_RM_recordSize = f_buffer->record_size();
        f_datax = new unsigned char [f_RM_recordSize];
        f_datax1 = new unsigned char [f_RM_recordSize];
        f_datay = new uint8_t [(f_RM_recordSize*2)];
        
        //end:Katcp
        
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
        //Katcp
        MTINFO(mtlog,"Boffile set to: "<<f_boffile);
	    MTINFO(mtlog,"Server IP set to: "<<f_katcp_server);
	
        if(f_katcp_server == NULL)
	    {
    	    MTERROR(mtlog,"Roachmantis: Need a server as first argument or in the KATCP_SERVER variable");
    	    return 2;
	    }
	    else
        {
            MTINFO(mtlog,"RoachMantis: KATCP_SERVER set, now trying to connect...");
        }

	    f_katcp_fd= net_connect(f_katcp_server, 0, NETC_VERBOSE_ERRORS | NETC_VERBOSE_STATS);
        if(f_katcp_fd< 0)
        {
            MTINFO(mtlog,"Unable to connect to"<<f_katcp_server);
            return 2;
        }
        else
        {
            MTINFO(mtlog,"Connected to roach board!");
        }
    
        if(f_katcp_cmdline == NULL)
        {
            MTERROR(mtlog,"Unable to allocate state");
            return 2;
        }
        else
        {
            MTINFO(mtlog,"State allocated");
        }
    
        if(borph_prog(f_katcp_cmdline, f_boffile, f_RM_timeout) < 0)
        {
            MTERROR(mtlog,"Unable to program FPGA");
            return 2;
        }
        else
        {
            MTINFO(mtlog,"FPGA programmed with: "<<f_boffile);
        }
 
        if(borph_write(f_katcp_cmdline, const_cast<char*>("snap64_ctrl"), 0, 00, f_RM_timeout) < 0)
        {
            MTERROR(mtlog,"Unable to write to register - 'snap64_ctrl'");
            return 2;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl'");
        }
        if(borph_write(f_katcp_cmdline, const_cast<char*>("snap64_ctrl"), 0, 0111, f_RM_timeout) < 0)
        {
            MTERROR(mtlog,"Unable to write to register - 'snap64_ctrl'");
            return 2;
        }
        else
        {
            MTINFO(mtlog,"Wrote - 'snap64_ctrl'");
        }
        //end:Katcp
        MTINFO(mtlog,"Next: 'snap64_bram_msb' and 'snap64_bram_msb' will be read");
        return true;
    }
    void digitizer_roach::execute()
    {
        iterator t_it( f_buffer );

        timespec t_live_start_time;
        timespec t_live_stop_time;
        timespec t_dead_start_time;
        timespec t_dead_stop_time;
        timespec t_stamp_time;

        //MTINFO( mtlog, "waiting" );

        f_condition->wait();

        MTINFO( mtlog, "loose at <" << t_it.index() << ">" );

        int t_old_cancel_state;
        pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &t_old_cancel_state );

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

            // slow things down a bit, since this is for testing purposes, after all
            usleep( 100 );
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
        int f_katcp_fd;
        if(borph_read(f_katcp_cmdline, const_cast<char*>("snap64_bram_msb"), f_datax, f_RM_recordSize, f_RM_timeout) < 0)
        {
            MTERROR(mtlog,"Unable to read register 'snap64_bram_msb'");
            return 2;
        }
        else
        {
            //MTINFO(mtlog,"Read - 'snap64_bram_msb'");
        }
        //printf("%d \n",(uint8_t)(f_datax[1]));
    
        if(borph_read(f_katcp_cmdline, const_cast<char*>("snap64_bram_lsb"), f_datax1, f_RM_recordSize, f_RM_timeout) < 0)
        {
            MTERROR(mtlog,"Unable to read register 'snap64_bram_lsb'");
            return 2;
        }   
        else
        {
            //MTINFO(mtlog,"Read - 'snap64_bram_lsb'");
        }
        for(int RM_count = 0; RM_count<(f_RM_recordSize); RM_count+=2)
        {
            f_datay[RM_count*2] = (uint8_t)(f_datax[RM_count]);
            f_datay[(RM_count*2)+1] = (uint8_t)(f_datax1[RM_count]);
        }
        //End:Katcp
        
        a_block->set_record_id( f_record_count );
        a_block->set_acquisition_id( f_acquisition_count );
        get_time_monotonic( &a_stamp_time );
        a_block->set_timestamp( time_to_nsec( a_stamp_time ) );
        ::memcpy( a_block->data_bytes(), f_datay, f_buffer->record_size() );
        ++f_record_count;
        return true;
    }
    bool digitizer_roach::stop()
    {
        ++f_acquisition_count;
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

    block_cleanup_roach::block_cleanup_roach( digitizer_roach::data_type* a_data ) :
            block_cleanup(),
            f_triggered( false ),
            f_data( a_data )
    {}
    block_cleanup_roach::~block_cleanup_roach()
    {}
    bool block_cleanup_roach::delete_data()
    {
        if( f_triggered ) return true;
        delete [] f_data;
        return true;
    }

}
