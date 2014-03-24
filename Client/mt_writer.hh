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
    class param_node;

    class writer :
        public callable
    {
        public:
            writer();
            virtual ~writer();

            void set_buffer( buffer* a_buffer, condition* a_condition );

            virtual void configure( const param_node* config ) = 0;

            bool initialize( request* a_response );
            virtual bool initialize_derived( request* a_response ) = 0;
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
            condition f_cancel_condition;

            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;

            virtual bool write( block* a_block ) = 0;
    };

#define MT_REGISTER_WRITER(writer_class, writer_name) \
        static registrar< writer, writer_class > s_##writer_class##_writer_registrar( writer_name );
#define MT_REGISTER_WRITER_NS(writer_namespace, writer_class, writer_name) \
        static registrar< writer, writer_namespace::writer_class > s_##writer_class##_writer_registrar( writer_name );

}

#endif
