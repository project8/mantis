#ifndef MT_DIGITIZER_TEST16_HH_
#define MT_DIGITIZER_TEST16_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_condition.hh"
#include "mt_mutex.hh"
#include "request.pb.h"

#include "thorax.hh"

#include <stdint.h>

//#include <semaphore.h>

namespace mantis
{
    class block_cleanup_test16;

    class digitizer_test16 :
        public digitizer
    {
        public:
            typedef uint16_t data_type;

            static unsigned bit_depth_test();
            static unsigned data_type_size_test();

        public:
            digitizer_test16();
            virtual ~digitizer_test16();

            bool allocate( buffer* a_buffer, condition* a_condition );
            bool initialize( request* a_request );
            void execute();
            void cancel();
            void finalize( response* a_response );

            bool write_mode_check( request_file_write_mode_t mode );

            unsigned bit_depth();
            unsigned data_type_size();

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

        private:
            static const unsigned s_bit_depth;
            static const unsigned s_data_type_size;

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
    };

    class block_cleanup_test16 : public block_cleanup
    {
        public:
            block_cleanup_test16( digitizer_test16::data_type* a_data );
            virtual ~block_cleanup_test16();
            virtual bool delete_data();
        private:
            bool f_triggered;
            digitizer_test16::data_type* f_data;
    };


    class test_digitizer_test16 : public test_digitizer
    {
        public:
            test_digitizer_test16() {}
            virtual ~test_digitizer_test16() {}

            bool run_test()
            {
                return true;
            }
    };

}

#endif
