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
    class run_description;

    /*!
     @class writer
     @author D. Furse, N. Oblath

     @brief Base class for writers

     @details
     This base class defines the generic writer interface.  Typical usage of a writer proceeds as follows:
      1. Create the writer
      2. Set the buffer and buffer condition (writer::set_buffer)
      3. Configure the writer (writer::configure)
      4. Initialize the writer (writer::initialize); writer is now ready for use
      5. Execute the buffer (typically done by starting a thread, via writer::execute)
      6. Finish up (writer::finalize)

     Subclasses of writer must implement the configure and initialize_derived functions to properly prepare
     the writer for use.  They also must implement the private function write to perform the action
     of the writer.

     Some writers may have specific actions that must be taken to properly prepare the writer for use.  These
     writers cannot be used in a completely generic way (e.g. file_writer).

     Be sure to check each writer's documentation for any necessary usage details.
    */
    class writer : public callable
    {
        public:
            writer();
            virtual ~writer();

            void set_buffer( buffer* a_buffer, condition* a_condition );

            bool initialize( run_description* a_run_desc );
            virtual bool initialize_derived( run_description* a_run_desc ) = 0;

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
