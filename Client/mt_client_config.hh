/*
 * mt_client_config.hh
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#ifndef MT_CLIENT_CONFIG_HH_
#define MT_CLIENT_CONFIG_HH_

#include "mt_param.hh"

namespace mantis
{

    class MANTIS_API client_config : public param_node
    {
        public:
            client_config();
            virtual ~client_config();
    };

} /* namespace mantis */
#endif /* MT_CLIENT_CONFIG_HH_ */