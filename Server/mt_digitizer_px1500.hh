#ifndef MT_DIGITIZER_PX1500_HH_
#define MT_DIGITIZER_PX1500_HH_

#include "mt_callable.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "px1500.h"
#include "thorax.hh"

namespace mantis
{

    class digitizer :
        public callable
    {
        public:
            digitizer( buffer* a_buffer, condition* a_condition );
            virtual ~digitizer();

            void initialize( request* a_request );
            void execute();
            void finalize( response* a_response );

        private:
            buffer* f_buffer;
            condition* f_condition;
            HPX4 f_handle;

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
