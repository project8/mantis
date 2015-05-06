/*
 * mt_run_server.hh
 *
 *  Created on: May 6, 2015
 *      Author: nsoblath
 */

#ifndef SERVER_MT_RUN_SERVER_HH_
#define SERVER_MT_RUN_SERVER_HH_

#include "mt_param.hh"

namespace mantis
{

    class MANTIS_API run_server
    {
        public:
            run_server( const param_node& a_node, const std::string& a_exe_name );
            ~run_server();

            void execute();

            int get_return() const;

        private:
            param_node f_config;
            std::string f_exe_name;

            int f_return;
    };

    inline int run_server::get_return() const
    {
        return f_return;
    }

} /* namespace mantis */

#endif /* SERVER_MT_RUN_SERVER_HH_ */
