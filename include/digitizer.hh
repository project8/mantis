#ifndef DIGITIZER_HH_
#define DIGITIZER_HH_

#include "callable.hh"

#include "types.hh"
#include "buffer.hh"
#include "condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "px1500.h"

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

            record_id_t f_record_last;
            record_id_t f_record_count;
            acquisition_id_t f_acquisition_count;
            timestamp_t f_live_time;
            timestamp_t f_dead_time;

            bool start();
            bool acquire( block* a_block );
            bool stop();
    };

}

#endif
