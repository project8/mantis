/////////////////////////////////////////////////
// Program to make egg files from roach1 board //
// Original Author: N.S.Oblath                 //
//		            nsoblath@mit.edu           //
// Modified by:     Prajwal Mohanmurthy        //
//                  prajwal@mohanmurthy.com    //
//		            MIT LNS                    //
/////////////////////////////////////////////////
#ifndef MT_DIGITIZER_ROACH_HH_
#define MT_DIGITIZER_ROACH_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_condition.hh"
#include "mt_mutex.hh"
#include "request.pb.h"
#include "mt_param.hh"

#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include <sys/select.h>
#include <sys/time.h>

#include "netc.h"
#include "katcp.h"
#include "katcl.h"

namespace mantis
{
    class block_cleanup_roach;

    class digitizer_roach : public digitizer
    {
        public:
            typedef uint8_t data_type;

            static unsigned data_type_size_roach();

        public:
            digitizer_roach();

            virtual void configure( const param_node* config );

            virtual ~digitizer_roach();

            bool allocate( buffer* a_buffer, condition* a_condition );
            bool initialize( request* a_request );
            void execute();
            void cancel();
            void finalize( response* a_response );

            bool write_mode_check( request_file_write_mode_t mode );

            unsigned data_type_size();

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

        public:
            bool run_basic_test()
            {
                // TODO: implement basic test
                return true;
            }

        private:
            static const unsigned s_data_type_size;

            int dispatch_client( char *msgname, int verbose);
            int borph_write( const std::string& regname, int buffer, int len);
            int borph_prog( const std::string& a_bof_file );
            int borph_read( const std::string& regname, void *buffer, int len );

            // katcp communication
            std::string f_katcp_server;
            struct katcl_line *f_katcp_cmdline;
            int f_katcp_fd;

            //Place to set vitals
            unsigned f_rm_half_record_size; // must be = 65536*4
            unsigned f_rm_timeout; // = 5000; /*Time out in ms*/
            std::string f_bof_file; //= const_cast<char*>("adc.bof");

            // arrays for reading data
            data_type* f_datax0;
            data_type* f_datax1;

            // hard-coded command strings
            char f_write_start[32],  f_write_end[32];
            char f_prog_start[32],   f_prog_end[32];
            char f_read_start[32],   f_read_end[32];
            std::string f_reg_name_msb, f_reg_name_lsb, f_reg_name_ctrl;

            //sem_t* f_semaphore;

            bool f_allocated;

            buffer* f_buffer;
            condition* f_condition;

            record_id_type f_record_last;
            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            atomic_bool f_canceled;
            condition f_cancel_condition;

            bool start();
            bool acquire( block* a_block, timespec& a_time_stamp );
            bool stop();

    };

    class block_cleanup_roach : public block_cleanup
    {
        public:
            block_cleanup_roach( digitizer_roach::data_type* a_memblock );
            virtual ~block_cleanup_roach();
            virtual bool delete_memblock();
        private:
            bool f_triggered;
            digitizer_roach::data_type* f_memblock;
    };

}

#endif
