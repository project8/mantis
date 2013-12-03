#ifndef MT_DIGITIZER_TEST_HH_
#define MT_DIGITIZER_TEST_HH_

#include "mt_digitizer.hh"

#include "thorax.hh"

namespace mantis
{
    class block;

    class digitizer_test :
        public digitizer
    {
        public:
            digitizer_test();
            virtual ~digitizer_test();

            void allocate( buffer* a_buffer, condition* a_condition );
            void initialize( request* a_request );
            void execute();
            void cancel();
            void finalize( response* a_response );

            bool write_mode_check( request_file_write_mode_t mode );

        private:
            bool f_allocated;

            buffer* f_buffer;
            condition* f_condition;

            record_id_type f_record_last;
            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            bool start();
            bool acquire( block* a_block, timespec& a_time_stamp );
            bool stop();
    };
}

#endif
