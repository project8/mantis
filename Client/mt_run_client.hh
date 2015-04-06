/*
 * mt_run_client.hh
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#ifndef MT_RUN_CLIENT_HH_
#define MT_RUN_CLIENT_HH_

//#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_param.hh"

namespace mantis
{
    class broker;
    //class client_file_writing;
    class connection;
    //class run_context_dist;

    // run_client was formerly used in a separate thread, hence the previous use of the callable base class

    class MANTIS_API run_client// : public callable
    {
        public:
            run_client( const param_node& a_node, const std::string& a_exe_name, const std::string& a_exchange );
            virtual ~run_client();

            void execute();
            //void cancel();

            int get_return();

        private:
            bool do_run_request( std::string& a_request_str, std::string& a_consumer_tag, std::string& a_reply_to );
            bool do_get_request( std::string& a_request_str, std::string& a_consumer_tag, std::string& a_reply_to, param_node& a_save_node );
            bool do_config_request( std::string& a_request_str, std::string& a_consumer_tag, std::string& a_reply_to );

            param_node f_config;
            std::string f_exe_name;
            std::string f_exchange;
            //atomic_bool f_canceled;
            int f_return;
    };

} /* namespace mantis */

#endif /* MT_RUN_CLIENT_HH_ */
