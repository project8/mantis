#ifndef MT_BIT_SHIFT_MODIFIER_HH_
#define MT_BIT_SHIFT_MODIFIER_HH_

#include "mt_modifier.hh"

#include "thorax.hh"

#include <string>

namespace mantis
{
    class run_description;

    /*!
     @class bit_shift_modifier
     @author N. Oblath

     @brief Modifier class for shifting data bits

     @details
     Configuration (via bit_shift_modifier::configure):
      * Details on the digitizer parameters are used to fill in  values in the egg-file header.
        If these are not supplied, default values are used (defaults are  the correct values for the px1500 digitizer).
    */
    class bit_shift_modifier : public modifier
    {
        public:
            bit_shift_modifier();
            virtual ~bit_shift_modifier();

            void configure( const param_node* );

            void set_run_description( run_description* a_run_desc );

            bool initialize_derived( request* a_response );

        private:
            struct dig_calib_params f_dig_params;

            run_description* f_run_desc;

            bool modify( block* a_block );
    };
}

#endif
