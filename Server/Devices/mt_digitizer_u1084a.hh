#ifndef MT_DIGITIZER_U1084A_HH_
#define MT_DIGITIZER_U1084A_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_condition.hh"

#define _LINUX 1
#include "AgMD1.h"
#include "AgMD1Fundamental.h"

//#include <semaphore.h>

namespace mantis
{
    void PrintU1084AError( ViSession a_handle, ViStatus a_status, const std::string& a_prepend_msg );

    class block_cleanup_u1084a;

    class digitizer_u1084a : public digitizer
    {
        public:
            typedef ViInt8 data_type;

            static unsigned data_type_size_u1084a();

        public:
            digitizer_u1084a();
            virtual ~digitizer_u1084a();

            bool allocate( buffer* a_buffer, condition* a_condition );
            bool deallocate( buffer* a_buffer );

            bool initialize( request* a_request );
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

            ViSession f_handle;
            bool f_allocated;
            unsigned f_postfix_size;

            buffer* f_buffer;
            condition* f_condition;

            time_nsec_type f_start_time;

            record_id_type f_record_last;
            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            atomic_bool f_canceled;
            condition f_cancel_condition;

            //ViInt32 f_number_samples;
            //block* f_block;

            bool start();
            bool acquire( block* a_block, timespec& a_time_stamp );
            bool stop();
    };


    class block_cleanup_u1084a : public block_cleanup
    {
        public:
            block_cleanup_u1084a( byte_type* a_memblock );
            virtual ~block_cleanup_u1084a();
            virtual bool delete_memblock();
        private:
            bool f_triggered;
            byte_type* f_memblock;
    };


}

#endif
