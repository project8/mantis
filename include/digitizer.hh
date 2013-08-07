#ifndef DIGITIZER_HH_
#define DIGITIZER_HH_

#include "callable.hh"

#include "types.hh"
#include "buffer.hh"
#include "mutex.hh"
#include "px1500.h"

namespace mantis
{

    class digitizer :
        public callable
    {
        public:
            digitizer( buffer* a_buffer, condition* a_condition );
            virtual ~digitizer();

            void initialize( run* a_run );
            void execute();
            void finalize( run* a_run );

        private:
            buffer* f_buffer;
            condition* f_condition;
            HPX4 f_handle;

            record_id_t f_last;
            double f_rate;
            double f_duration;

            record_id_t f_record_count;
            acquisition_id_t f_acquisition_count;
            timestamp_t f_live_time;
            timestamp_t f_dead_time;

            bool start();
            bool acquire( data_t* a_block );
            bool stop();
    };

}

#endif
