#ifndef MT_DIGITIZER_PXIE5122_HH_
#define MT_DIGITIZER_PXIE5122_HH_

#include "mt_digitizer.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_condition.hh"
#include "mt_mutex.hh"

#include "niScope.h"

#include <stdint.h>

//#include <semaphore.h>

namespace mantis
{
    class block_cleanup_pxie5122;

    /* Available configuration values
    -input impedance( "input-impedance" )
    - minimum sample rate( "rate-req" )
    - minimum number of points per record( "block-size-req" )
    - voltage range( "voltage-range" )
    - voltage offset( "voltage-offset" )
    - coupling( "input-coupling" )
    - probe attenuation( "probe-attenuation" )
    */


    class digitizer_pxie5122 : public digitizer
    {
        public:
            typedef ViInt16 data_type;

            static unsigned data_type_size_test();

        public:
            digitizer_pxie5122();
            virtual ~digitizer_pxie5122();

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

            bool handle_error( ViStatus a_status );

            //sem_t* f_semaphore;

            ViSession f_handle;
            std::string f_resource_name;

            bool f_allocated;

            niScope_wfmInfo f_waveform_info;

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


    class block_cleanup_pxie5122 : public block_cleanup
    {
        public:
            block_cleanup_pxie5122( byte_type* a_memblock );
            virtual ~block_cleanup_pxie5122();
            virtual bool delete_memblock();
        private:
            bool f_triggered;
            byte_type* f_memblock;
    };

}

#endif /* MT_DIGITIZER_PXIE5122_HH_ */