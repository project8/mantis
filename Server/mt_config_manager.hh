/*
 * mt_config_manager.hh
 *
 *  Created on: May 4, 2015
 *      Author: nsoblath
 */

#ifndef SERVER_MT_CONFIG_MANAGER_HH_
#define SERVER_MT_CONFIG_MANAGER_HH_

#include "mt_mutex.hh"
#include "mt_param.hh"

namespace mantis
{
    class device_manager;
    struct request_reply_package;

    class config_manager
    {
        public:
            config_manager( const param_node& a_config, device_manager* a_dev_mgr );
            ~config_manager();

            param_node* copy_master_server_config( const std::string& a_node_name = "" ) const;

            bool handle_get_acq_config_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool handle_get_server_config_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

            bool handle_set_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

            bool handle_replace_acq_config( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

            bool handle_add_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool handle_remove_request( const param_node& a_msg_payload, const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

        private:
            mutable mutex f_msc_mutex;
            param_node f_master_server_config;

            device_manager* f_dev_mgr;
    };

} /* namespace mantis */

#endif /* SERVER_MT_CONFIG_MANAGER_HH_ */
