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

    /*
    Available configuration values:
    - input impedance (not yet; niScope_ConfigureChanCharacteristics)
    - max input frequency? (not yet; -1 is full bandwidth; 0 is use hardware defaults; niScope_ConfigureChanCharacteristics)
    - minimum sample rate (not yet; niScope_ConfigureHorizontalTiming)
    - minimum number of points per record (not yet; niScope_ConfigureHorizontalTiming)
    - number of records? (not yet; niScope_ConfigureHorizontalTiming)
    - voltage range (not yet; niScope_ConfigureVertical)
    - voltage offset (not yet; niScope_ConfigureVertical)
    - coupling (not yet; niScope_ConfigureVertical)
    - probe attenuation (not yet; niScope_ConfigureVertical)
    - various trigger things (not yet; niScope_ConfigureTrigger...)
    */

    /*
    Information to obtain
    - Actual number of samples (not yet; niScope_ActualMeasWfmSize)
    - Actual number of waveforms (not yet; niScope_ActualNumWfms)
    - Actual record length (not yet; niScope_ActualRecordLength)
    - Actual sample mode (not yet; niScope_SampleMode)
    - Actual sample rate (not yet; niScope_SampleRate)
    */

    class digitizer_pxie5122 : public digitizer
    {
        public:
            typedef ViInt16 data_type;

            static unsigned data_type_size_test();

        public:
            digitizer_pxie5122();
            virtual ~digitizer_pxie5122();

            bool allocate( buffer* a_buffer, condition* a_condition );
            bool deallocate( buffer* a_buffer );

            bool initialize( const param_node* a_config );
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

            bool f_allocated;

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

            bool allocate();
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
