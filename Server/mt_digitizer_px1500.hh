#ifndef MT_DIGITIZER_PX1500_HH_
#define MT_DIGITIZER_PX1500_HH_

#include "mt_digitizer.hh"

#include "px1500.h"
#include "thorax.hh"

//#include <semaphore.h>

namespace mantis
{
    class block;

    class digitizer_px1500 :
        public digitizer
    {
        public:
            digitizer_px1500();
            virtual ~digitizer_px1500();

            void allocate( buffer* a_buffer, condition* a_condition );
            void initialize( request* a_request );
            void execute();
            void cancel();
            void finalize( response* a_response );

            bool write_mode_check( request_file_write_mode_t mode );

        private:
            //sem_t* f_semaphore;

            HPX4 f_handle;
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
