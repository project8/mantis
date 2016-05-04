/*
 * mt_config_manager.hh
 *
 *  Created on: May 4, 2015
 *      Author: nsoblath
 */

#ifndef SERVER_MT_CONFIG_MANAGER_HH_
#define SERVER_MT_CONFIG_MANAGER_HH_

#include "mt_mutex.hh"

#include "hub.hh"

#include "param.hh"

namespace mantis
{
    class device_manager;
    class msg_request;
    struct request_reply_package;

    class config_manager
    {
        public:
            config_manager( const scarab::param_node& a_config, device_manager* a_dev_mgr );
            ~config_manager();

            scarab::param_node* copy_master_server_config( const std::string& a_node_name = "" ) const;

            bool handle_get_acq_config_request( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );
            bool handle_get_server_config_request( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );

            bool handle_set_request( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );

            bool handle_replace_acq_config( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );

            bool handle_add_request( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );
            bool handle_remove_request( const dripline::request_ptr_t a_request, dripline::reply_package& a_reply_pkg );

        private:
            mutable mutex f_msc_mutex;
            scarab::param_node f_master_server_config;

            device_manager* f_dev_mgr;
    };

} /* namespace mantis */

#endif /* SERVER_MT_CONFIG_MANAGER_HH_ */
