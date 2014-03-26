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
            
            //From Katcp
            static int dispatch_client(struct katcl_line *l, char *msgname, int verbose, unsigned int timeout);
            int borph_write(struct katcl_line *l, char *regname, int buffer, int len, unsigned int timeout);
            int borph_prog(struct katcl_line *l, char *boffile, unsigned int timeout);
            int borph_read(struct katcl_line *l, char *regname, void *buffer, int len, unsigned int timeout);
            //End:Katcp part//

        private:
            static const unsigned s_data_type_size;
            
            //For Katcp
            char *f_katcp_server;
	        struct katcl_line *f_katcp_cmdline;
	        int f_katcp_fd;
	
	        //Place to set vitals
	        unsigned int f_RM_recordSize; // = 65536*4;
	        int f_RM_timeout; // = 5000; /*Time out in ms*/
	        char *f_boffile; //= const_cast<char*>("adc.bof");
	        //
	
	        unsigned char* f_datax;
	        unsigned char* f_datax1;
	        uint8_t* f_datay;
	        //End Katcp Desc.//
	        
            //sem_t* f_semaphore;

            data_type* f_master_record;

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
            
            //For Roach cmd line parameters
            std::string f_roach_ipaddress;
            std::string f_roach_boffile;
    };

    class block_cleanup_roach : public block_cleanup
    {
        public:
            block_cleanup_roach( digitizer_roach::data_type* a_data );
            virtual ~block_cleanup_roach();
            virtual bool delete_data();
        private:
            bool f_triggered;
            digitizer_roach::data_type* f_data;
    };


    class test_digitizer_roach : public test_digitizer
    {
        public:
            test_digitizer_roach() {}
            virtual ~test_digitizer_roach() {}

            bool run_test()
            {
                return true;
            }
    };

}

#endif
