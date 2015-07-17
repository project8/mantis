/*
 * mt_broker.hh
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#ifndef MT_BROKER_HH_
#define MT_BROKER_HH_

#include "mt_amqp.hh"
#include "mt_constants.hh"

#include <string>

namespace mantis
{
    class connection;



    class MANTIS_API broker
    {
        public:
            broker( const std::string& a_address, unsigned port );
            ~broker();

            amqp_channel_ptr open_channel() const;

            const std::string& get_address() const;
            unsigned get_port() const;

        private:
            std::string f_address;
            unsigned f_port;
    };

    inline const std::string& broker::get_address() const
    {
        return f_address;
    }
    inline unsigned broker::get_port() const
    {
        return f_port;
    }


} /* namespace mantis */

#endif /* MT_BROKER_HH_ */
