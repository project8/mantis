#ifndef MT_WRITER_HH_
#define MT_WRITER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "thorax.hh"

namespace mantis
{
    class configurator;

    class writer :
        public callable
    {
        public:
            writer();
            virtual ~writer();

            void set_buffer( buffer* a_buffer, condition* a_condition );

            virtual void configure( configurator* config ) = 0;

            virtual void initialize( request* a_response );
            void execute();
            void cancel();
            virtual void finalize( response* a_response );

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

        protected:
            buffer* f_buffer;
            condition* f_condition;

            atomic_bool f_canceled;

            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;

            virtual bool write( block* a_block ) = 0;
    };
}

#endif
