#ifndef MT_WRITER_HH_
#define MT_WRITER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "thorax.hh"

namespace mantis
{
    class param_node;
    class acq_request;

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
    class MANTIS_API writer : public callable
    {
        public:
            writer();
            virtual ~writer();

            void set_buffer( buffer* a_buffer, condition* a_condition );

            bool initialize( acq_request* a_run_desc );
            virtual bool initialize_derived( acq_request* a_run_desc ) = 0;

            void execute();

            void cancel();
            void finalize( param_node* a_response );
            virtual void finalize_derived( param_node* a_response ) = 0;

            // thread-safe getter
            bool get_canceled();
            // thread-safe setter
            void set_canceled( bool a_flag );

            enum status {
                k_ok = 0,
                k_warning = 1,
                k_error = 2
            };
            status get_status() const;
            const std::string& get_status_message() const;
            void set_status( status a_status, const std::string& a_message );

        protected:
            buffer* f_buffer;
            condition* f_condition;

            atomic_bool f_canceled;
            condition f_cancel_condition;

            record_id_type f_record_count;
            acquisition_id_type f_acquisition_count;
            time_nsec_type f_live_time;

            status f_status;
            std::string f_status_message;

            virtual bool write( block* a_block ) = 0;
    };

    inline writer::status writer::get_status() const {
        return f_status;
    }

    inline const std::string& writer::get_status_message() const {
        return f_status_message;
    }

    inline void writer::set_status( writer::status a_status, const std::string& a_message )
    {
        f_status = a_status;
        f_status_message = a_message;
        return;
    }

#define MT_REGISTER_WRITER(writer_class, writer_name) \
        static registrar< writer, writer_class > s_##writer_class##_writer_registrar( writer_name );
#define MT_REGISTER_WRITER_NS(writer_namespace, writer_class, writer_name) \
        static registrar< writer, writer_namespace::writer_class > s_##writer_class##_writer_registrar( writer_name );

}

#endif
