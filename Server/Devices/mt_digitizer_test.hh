#ifndef MT_DIGITIZER_TEST_HH_
#define MT_DIGITIZER_TEST_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_condition.hh"
#include "mt_mutex.hh"

#include <stdint.h>

//#include <semaphore.h>

namespace mantis
{
    class block_cleanup_test;

    struct MANTIS_API digitizer_test_config_template : digitizer_config_template
    {
        void add( param_node* a_node, const std::string& a_type );
    };

    class MANTIS_API digitizer_test :
        public digitizer
    {
        public:
            typedef uint8_t data_type;

            static unsigned data_type_size_test();

        public:
            digitizer_test();
            virtual ~digitizer_test();

            bool allocate();
            bool deallocate();

            bool initialize( param_node* a_global_config, param_node* a_dev_config );
            void execute();
            void cancel();
            void finalize( param_node* a_response );

            unsigned data_type_size();

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

        public:
            bool run_basic_test();

        private:
            static const unsigned s_data_type_size;

            //sem_t* f_semaphore;

            data_type* f_master_record;

            bool f_allocated;

            time_nsec_type f_start_time;

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


    class MANTIS_API block_cleanup_test : public block_cleanup
    {
        public:
            block_cleanup_test( byte_type* a_memblock );
            virtual ~block_cleanup_test();
            virtual bool delete_memblock();
        private:
            bool f_triggered;
            byte_type* f_memblock;
    };

}

#endif
