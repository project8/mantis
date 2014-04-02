/*
 * mt_oversampling_modifier.hh
 *
 *  Created on: Apr 2, 2014
 *      Author: nsoblath
 */

#ifndef MT_OVERSAMPLING_MODIFIER_HH_
#define MT_OVERSAMPLING_MODIFIER_HH_

#include "mt_modifier.hh"

namespace mantis
{

    class oversampling_modifier : public modifier
    {
        public:
            oversampling_modifier();
            virtual ~oversampling_modifier();

            void configure( const param_node* );

            bool initialize_derived( request* a_response );

        private:


            bool modify( block* a_block );

    };

} /* namespace mantis */

#endif /* MT_OVERSAMPLING_MODIFIER_HH_ */
