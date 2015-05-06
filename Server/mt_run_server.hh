/*
 * mt_run_server.hh
 *
 *  Created on: May 6, 2015
 *      Author: nsoblath
 */

#ifndef SERVER_MT_RUN_SERVER_HH_
#define SERVER_MT_RUN_SERVER_HH_

#include "mt_atomic.hh"
#include "mt_param.hh"

namespace mantis
{

    class MANTIS_API run_server
    {
        public:
            run_server( const param_node& a_node, const std::string& a_exe_name );
            virtual ~run_server();

            void execute();

            int get_return() const;

        private:
            param_node f_config;
            std::string f_exe_name;

            int f_return;

        public:
            enum status
            {
                k_initialized = 0,
                k_starting = 1,
                k_running = 5,
                k_done = 10,
                k_error = 100
            };

            static std::string interpret_status( status a_status );

            status get_status() const;
            void set_status( status a_status );

        private:
            boost::atomic< status > f_status;

    };

    inline int run_server::get_return() const
    {
        return f_return;
    }

    inline run_server::status run_server::get_status() const
    {
        return f_status.load();
    }

    inline void run_server::set_status( status a_status )
    {
        f_status.store( a_status );
        return;
    }

} /* namespace mantis */

#endif /* SERVER_MT_RUN_SERVER_HH_ */
