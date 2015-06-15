/*
 * mt_authentication.hh
 *
 *  Created on: Jun 15, 2015
 *      Author: nsoblath
 */

#ifndef MT_AUTHENTICATION_HH_
#define MT_AUTHENTICATION_HH_

#include "mt_param.hh"
#include "mt_singleton.hh"

#include "mt_constants.hh"

namespace mantis
{

    class MANTIS_API authentication : public singleton< authentication >, public param_node
    {
        public:
            const std::string& get_auth_filename() const;
            bool is_loaded() const;

        private:
            std::string f_auth_filename;
            bool f_is_loaded;

        protected:
            friend class singleton< authentication >;
            friend class destroyer< authentication >;
            authentication();
            ~authentication();

    };

    inline const std::string& authentication::get_auth_filename() const
    {
        return f_auth_filename;
    }

    inline bool authentication::is_loaded() const
    {
         return f_is_loaded;
    }

} /* namespace mantis */

#endif /* MT_AUTHENTICATION_HH_ */
