/*
 * mt_client_config.hh
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#ifndef MT_CLIENT_CONFIG_HH_
#define MT_CLIENT_CONFIG_HH_

#include "mt_configuration.hh"

namespace mantis
{

    class client_config : public configuration
    {
        public:
            client_config();
            virtual ~client_config();
    };

} /* namespace Katydid */
#endif /* MT_CLIENT_CONFIG_HH_ */
