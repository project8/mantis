/*
 * mt_client_config.hh
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#ifndef MT_SERVER_CONFIG_HH_
#define MT_SERVER_CONFIG_HH_

#include "mt_api.hh"

#include "param.hh"

namespace mantis
{

    class MANTIS_API server_config : public scarab::param_node
    {
        public:
            server_config();
            virtual ~server_config();
    };

} /* namespace mantis */
#endif /* MT_SERVER_CONFIG_HH_ */
