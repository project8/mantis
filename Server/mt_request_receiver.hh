#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_mutex.hh"
#include "mt_param.hh"

#include "SimpleAmqpClient/Envelope.h"

namespace mantis
{
    class acq_request_db;
    class broker;
    class buffer;
    class condition;
    class config_manager;
    class connection;
    class device_manager;
    class run_server;
    class server_worker;

    class request_receiver;

    struct MANTIS_API request_reply_package
    {
        AmqpClient::Envelope::ptr_t f_envelope;
        param_node& f_reply_node;
        const request_receiver* f_request_receiver;
        request_reply_package( AmqpClient::Envelope::ptr_t a_envelope, param_node& a_reply_node, const request_receiver* a_req_recvr ) :
            f_envelope( a_envelope ),
            f_reply_node( a_reply_node ),
            f_request_receiver( a_req_recvr )
        {}
        bool send_reply( unsigned a_return_code, const std::string& a_return_msg );
    };


    class MANTIS_API request_receiver : public callable
    {
        public:
            request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db, server_worker* a_server_worker );
            virtual ~request_receiver();

            void execute();
            void cancel();

        private:
            friend struct request_reply_package;

            bool do_run_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool do_get_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool do_set_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );
            bool do_cmd_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

            bool send_reply( unsigned a_return_code, const std::string& a_return_msg, request_reply_package& a_pkg ) const;

            broker* f_broker;
            std::string f_queue_name;
            std::string f_consumer_tag;

            run_server* f_run_server;
            config_manager* f_conf_mgr;
            acq_request_db* f_acq_request_db;
            server_worker* f_server_worker;

            atomic_bool f_canceled;

        public:
            enum status
            {
                k_initialized = 0,
                k_starting = 1,
                k_listening = 5,
                k_processing = 6,
                k_canceled = 9,
                k_done = 10,
                k_error = 100
            };

            static std::string interpret_status( status a_status );

            status get_status() const;
            void set_status( status a_status );

        private:
            boost::atomic< status > f_status;

    };

    inline request_receiver::status request_receiver::get_status() const
    {
        return f_status.load();
    }

    inline void request_receiver::set_status( status a_status )
    {
        f_status.store( a_status );
        return;
    }

}

#endif
