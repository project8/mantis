#include "mt_katcp.hh"

#include "mt_logger.hh"

#include "katcp.h"
#include "katcl.h"
#include "netc.h"

#include <errno.h>
#include <sys/time.h>
#include <vector>

using std::string;

namespace mantis
{
    MTLOGGER( mtlog, "katcp" );

    katcp::katcp() :
            f_server_ip(),
            f_cmd_line( NULL ),
            f_file_desc( -1 ),
            f_timeout( 5000 )
    {
    }

    katcp::~katcp()
    {
        destroy_katcl(f_cmd_line, 1);
    }

    bool katcp::connect()
    {
        // Connect to the ROACH board
        if( f_server_ip.empty() )
        {
            MTERROR( mtlog,"Please provide the host address for the ROACH system" );
            return false;
        }

        // get the file descriptor
        f_file_desc = net_connect( const_cast< char* >( f_server_ip.c_str() ), 0, NETC_VERBOSE_ERRORS | NETC_VERBOSE_STATS );
        if( f_file_desc < 0 )
        {
            MTINFO( mtlog,"Unable to connect to the ROACH board at <"<< f_server_ip << ">" );
            return false;
        }

        MTINFO( mtlog,"Connected to ROACH board at <" << f_server_ip << ">" );

        // katcp command line
        destroy_katcl( f_cmd_line, 0 );
        f_cmd_line = create_katcl( f_file_desc );
        if( f_cmd_line == NULL )
        {
            MTERROR( mtlog, "Unable to allocate katcp command line" );
            return false;
        }

        return true;
    }

    int katcp::program_bof( const string& a_bof_file )
    {
        /* populate a request */
        if( append_string_katcl( f_cmd_line, KATCP_FLAG_FIRST, "?progdev" ) < 0 )
            return -1;
        if( append_string_katcl( f_cmd_line, KATCP_FLAG_LAST, const_cast< char* >( a_bof_file.c_str() ) ) < 0 )
            return -1;

        /* use above function to send request */
        if( dispatch_client( "!progdev", 1 ) < 0 )
            return -1;

        /* clean up request for next call */
        have_katcl( f_cmd_line );

        return 0;
    }

    int katcp::write_uint_to_reg( const string& a_regname, uint a_buffer, int a_length)
    {
        // populate a request
        if( append_string_katcl( f_cmd_line, KATCP_FLAG_FIRST, "?write" ) < 0)
            return -1;
        if( append_string_katcl( f_cmd_line, 0, const_cast< char* >( a_regname.c_str() ) ) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, 0, 0) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, 0, a_buffer) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, KATCP_FLAG_LAST, a_length) < 0 )
            return -1;

        // send the request
        if( dispatch_client( "!write" , 1 ) < 0 )
            return -1;

        // clean up the command line for the next call
        have_katcl( f_cmd_line );

        return 0;
    }

    int katcp::read_from_reg( const string& a_regname, void* a_buffer, int a_length )
    {
        // populate a request
        if( append_string_katcl( f_cmd_line, KATCP_FLAG_FIRST, "?read" ) < 0 )
            return -1;
        if( append_string_katcl( f_cmd_line, 0, const_cast< char* >( a_regname.c_str() ) ) < 0 )
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, 0, 0 ) < 0 )
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, KATCP_FLAG_LAST, a_length ) < 0 )
            return -1;

        // send the request
        if( dispatch_client( "!read", 1 ) < 0 )
            return -1;

        // check that a response was received
        int t_count = arg_count_katcl( f_cmd_line );
        if( t_count < 2 )
        {
            MTERROR( mtlog,"insufficient arguments in reply" );
            return -1;
        }

        // get the data out; make sure it's the correct length
        int t_got = arg_buffer_katcl( f_cmd_line, 2, a_buffer, a_length );
        if( t_got < a_length )
        {
            MTERROR( mtlog,"partial data, wanted "<< a_length <<", got "<< t_got );
            return -1;
        }

        // clean up the command line for the next call
        have_katcl( f_cmd_line );

        return a_length;
    }

    int katcp::tap_start( const string& a_device, const string& a_device_mac, const string& a_device_ip, uint16_t a_device_port )
    {
        // prepare a request
        if( append_string_katcl( f_cmd_line, KATCP_FLAG_FIRST, "?tap-start" ) < 0 )
            return -1;
        if( append_string_katcl( f_cmd_line, 0, const_cast< char* >( a_device.c_str() ) ) < 0 )
            return -1;
        if( append_string_katcl( f_cmd_line, 0, const_cast< char* >( a_device_mac.c_str() ) ) < 0 )
            return -1;
        if( append_string_katcl( f_cmd_line, 0, const_cast< char* >( a_device_ip.c_str() ) ) < 0 )
            return -1;
        //if( append_args_katcl( f_cmd_line, 0, const_cast< char* >( katcp::mac_string_to_int( a_device_mac ).c_str() ),
        //                                      const_cast< char* >( katcp::ip_string_to_int(  a_device_ip  ).c_str() ) ) < 0)
            return -1;
        if( append_unsigned_long_katcl( f_cmd_line, KATCP_FLAG_LAST, a_device_port ) < 0 )
            return -1;

        // send the request
        if( dispatch_client( "!tap-start" , 1 ) < 0 )
            return -1;

        // clean up the command line for the next call
        have_katcl( f_cmd_line );

        return 0;
    }


    int katcp::dispatch_client( const char* a_msgname, int a_verbose )
    {
        fd_set fd_set_read, fd_set_write;
        struct timeval timeout_tv;
        int result;
        char *ptr, *match;
        int prefix;

        int cmd_fileno = fileno_katcl( f_cmd_line );

        if( a_msgname )
        {
            switch( a_msgname[0] )
            {
                case '!':
                case '?':
                    prefix = strlen( a_msgname + 1 );
                    match = a_msgname + 1;
                    break;
                default:
                    prefix = strlen( a_msgname );
                    match = a_msgname;
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

            FD_ZERO( &fd_set_read );
            FD_ZERO( &fd_set_write );

            if( match )
            { /* only look for data if we need it */
                FD_SET( cmd_fileno, &fd_set_read );
            }

            if( flushing_katcl( f_cmd_line ) )
            { /* only write data if we have some */
                FD_SET( cmd_fileno, &fd_set_write );
            }

            timeout_tv.tv_sec  = f_timeout / 1000;
            timeout_tv.tv_usec = ( f_timeout % 1000 ) * 1000;

            result = select( cmd_fileno + 1, &fd_set_read, &fd_set_write, NULL, &timeout_tv );
            switch( result )
            {
                case -1 :
                    switch ( errno )
                    {
                        case EAGAIN :
                        case EINTR  :
                            continue; /* WARNING */
                        default     :
                            return -1;
                    }
                    break;
                case  0 :
                    if( a_verbose )
                    {
                        MTERROR( mtlog, "dispatch: no io activity within " << f_timeout << " ms" );
                    }
                    return -1;
            }

            if( FD_ISSET( cmd_fileno, &fd_set_write ) )
            {
                result = write_katcl( f_cmd_line );
                if( result < 0 )
                {
                    MTERROR( mtlog, "dispatch: write failed:" << strerror( error_katcl( f_cmd_line ) ) );
                    return -1;
                }
                if( ( result > 0 ) && ( match == NULL ) )
                { /* if we finished writing and don't expect a match then quit */
                    return 0;
                }
            }

            if( FD_ISSET( cmd_fileno, &fd_set_read ) )
            {
                result = read_katcl( f_cmd_line );
                if( result )
                {
                    MTERROR( mtlog, "dispatch: read failed : " << strerror( error_katcl( f_cmd_line) ) << " : connection terminated" );
                    return -1;
                }
            }

            while( have_katcl( f_cmd_line ) > 0)
            {
                ptr = arg_string_katcl( f_cmd_line, 0 );
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
                            if( match )
                            {
                                if( strncmp( match, ptr + 1, prefix ) ||
                                        ( ( ptr[prefix + 1] != '\0' ) && ( ptr[prefix + 1] != ' ') ) )
                                {
                                    MTERROR( mtlog, "dispatch: warning, encountered reply " << ptr << " not match " << match );
                                }
                                else
                                {
                                    ptr = arg_string_katcl( f_cmd_line, 1 );
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


    const string& katcp::get_server_ip() const
    {
        return f_server_ip;
    }

    void katcp::set_server_ip( const string& a_ip )
    {
        f_server_ip = a_ip;
        return;
    }

    unsigned katcp::get_timeout() const
    {
        return f_timeout;
    }

    void katcp::set_timeout( unsigned a_timeout_ms )
    {
        f_timeout = a_timeout_ms;
        return;
    }

}
