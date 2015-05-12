/*
 * mt_broker.hh
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#ifndef MT_BROKER_HH_
#define MT_BROKER_HH_

#include "mt_singleton.hh"

#include "mt_constants.hh"

#include "SimpleAmqpClient/SimpleAmqpClient.h"

#include <string>

namespace mantis
{
    class connection;

    class MANTIS_API broker : public singleton< broker >
    {
        public:
            bool connect( const std::string& a_address, unsigned port );
            void disconnect();

            connection& get_connection();
            const connection& get_connection() const;

            bool is_connected() const;

            const std::string& get_address() const;
            unsigned get_port() const;

        protected:
            friend class singleton< broker >;
            friend class destroyer< broker >;
            broker();
            ~broker();

        private:
            std::string f_address;
            unsigned f_port;
            connection* f_connection;
    };

    inline connection& broker::get_connection()
    {
        return *f_connection;
    }

    inline const connection& broker::get_connection() const
    {
        return *f_connection;
    }

    inline bool broker::is_connected() const
    {
        return ! (f_connection == NULL);
    }

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
