#ifndef MT_DIGITIZER_TEST_HH_
#define MT_DIGITIZER_TEST_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_condition.hh"
#include "mt_mutex.hh"
#include "request.pb.h"

#include "thorax.hh"

//#include <semaphore.h>

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

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

        private:
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
}

#endif
