/*
 * mt_client_config.hh
 *
 *  Created on: Nov 4, 2013
 *      Author: nsoblath
 */

#ifndef MT_SERVER_CONFIG_HH_
#define MT_SERVER_CONFIG_HH_

#include "mt_config_node.hh"

namespace mantis
{

    class server_config : public config_value_object
    {
        public:
            server_config();
            virtual ~server_config();
    };

} /* namespace Katydid */
#endif /* MT_CLIENT_CONFIG_HH_ */
