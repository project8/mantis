/*
 * mt_configurator.hh
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_CONFIGURATOR_HH_
#define MT_CONFIGURATOR_HH_

#include "mt_configuration.hh"

namespace mantis
{

    class configurator
    {
        public:
            configurator( int an_argc, char** an_argv, configuration a_default = configuration() );
            virtual ~configurator();

        private:
            configuration f_master_config;
    };

} /* namespace mantis */
#endif /* MT_CONFIGURATOR_HH_ */
