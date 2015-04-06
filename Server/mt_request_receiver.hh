#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_mutex.hh"
#include "mt_param.hh"

#include "SimpleAmqpClient/Envelope.h"

namespace mantis
{
    class broker;
    class buffer;
    class condition;
    class connection;
    class device_manager;
    class run_database;
    class server_tcp;

    class MANTIS_API request_receiver : public callable
    {
        public:
            request_receiver( const param_node& a_config, device_manager* a_dev_mgr, run_database* a_run_database, condition* a_condition, const std::string& a_exe_name = "unknown" );
            virtual ~request_receiver();

            void execute();
            void cancel();

        private:
            void apply_config( const std::string& a_config_addr, const param_value& a_value );

            bool do_run_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope );
            bool do_get_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope );
            bool do_set_request( const param_node& a_msg_payload, AmqpClient::Envelope::ptr_t a_envelope );

            bool acknowledge_and_reply( const param_node& a_reply_node, unsigned a_return_code, AmqpClient::Envelope::ptr_t a_envelope );

            mutex f_msc_mutex;
            param_node f_master_server_config;

            broker* f_broker;
            std::string f_queue_name;
            std::string f_consumer_tag;

            device_manager* f_dev_mgr;
            run_database* f_run_database;
            condition* f_queue_condition;
            std::string f_exe_name;

            atomic_bool f_canceled;
    };

}

#endif
