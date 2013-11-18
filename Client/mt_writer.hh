#ifndef MT_WRITER_HH_
#define MT_WRITER_HH_

#include "mt_callable.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "thorax.hh"

namespace mantis
{

    class writer :
        public callable
    {
        public:
            writer( buffer* a_buffer, condition* a_condition );
            virtual ~writer();

            virtual void initialize( request* a_response );
            void execute();
            virtual void finalize( response* a_response );

        protected:
            buffer* f_buffer;
            condition* f_condition;

            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;

            virtual bool write( block* a_block ) = 0;
    };

}

#endif
