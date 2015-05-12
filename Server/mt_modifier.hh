#ifndef MT_MODIFIER_HH_
#define MT_MODIFIER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "thorax.hh"

namespace mantis
{
    class param_node;

    /*!
     @class modifier
     @author N. Oblath

     @brief Base class for modifiers

     @details
     This base class defines the generic modifier interface.  Typical usage of a modifier proceeds as follows:
      1. Create the modifier
      2. Set the buffer and buffer condition (modifier::set_buffer)
      3. Configure the modifier (modifier::configure)
      4. Initialize the modifier (modifier::initialize); modifier is now ready for use
      5. Execute the buffer (typically done by starting a thread, via modifier::execute)
      6. Finish up (modifier::finalize)

     Subclasses of modifier must implement the configure and initialize_derived functions to properly prepare
     the modifier for use.  They also must implement the private function write to perform the action
     of the modifier.

     Some modifiers may have specific actions that must be taken to properly prepare the modifier for use.  These
     modifiers cannot be used in a completely generic way (e.g. file_modifier).

     Be sure to check each modifier's documentation for any necessary usage details.
    */
    class MANTIS_API modifier : public callable
    {
        public:
            modifier();
            virtual ~modifier();

            void set_buffer( buffer* a_buffer, condition* a_condition );

            bool initialize( param_node* a_node );
            virtual bool initialize_derived( param_node* a_node ) = 0;
            void execute();
            void cancel();
            virtual void finalize( param_node* a_response );

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

            virtual bool modify( block* a_block ) = 0;
    };

#define MT_REGISTER_MODIFIER(modifier_class, modifier_name) \
        static registrar< modifier, modifier_class > s_##modifier_class##_modifier_registrar( modifier_name );

}

#endif
