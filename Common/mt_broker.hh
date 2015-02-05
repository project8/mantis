/*
 * mt_broker.hh
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#ifndef MT_BROKER_HH_
#define MT_BROKER_HH_

#include "mt_constants.hh"

#include "SimpleAmqpClient/SimpleAmqpClient.h"

#include <string>

namespace mantis
{
    class connection;

    class MANTIS_API broker
    {
        public:
            broker( const std::string& a_address, unsigned port );
            virtual ~broker();

            connection* create_connection();

        private:
            std::string f_address;
            unsigned f_port;
    };

} /* namespace mantis */

#endif /* MT_BROKER_HH_ */
