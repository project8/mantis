#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_amqp.hh"
#include "mt_atomic.hh"
#include "mt_mutex.hh"
#include "mt_param.hh"
#include "mt_uuid.hh"

#include "SimpleAmqpClient/Envelope.h"

namespace mantis
{
    class acq_request_db;
    class buffer;
    class condition;
    class config_manager;
    class device_manager;
    class msg_request;
    class run_server;
    class server_worker;

    class request_receiver;

    struct MANTIS_API request_reply_package
    {
        const msg_request* f_request;
        param_node f_payload;
        amqp_channel_ptr f_channel;
        request_reply_package( const msg_request* a_request, amqp_channel_ptr a_channel ) :
            f_request( a_request ),
            f_payload(),
            f_channel( a_channel )
        {}
        bool send_reply( unsigned a_return_code, const std::string& a_return_msg );
    };


    class MANTIS_API request_receiver : public callable
    {
        public:
            request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db, server_worker* a_server_worker, amqp_channel_ptr a_channel );
            virtual ~request_receiver();

            void execute();
            void cancel();

        private:
            friend struct request_reply_package;

            bool do_run_request( const msg_request* a_request, request_reply_package& a_reply_pkg );
            bool do_get_request( const msg_request* a_request, request_reply_package& a_reply_pkg );
            bool do_set_request( const msg_request* a_request, request_reply_package& a_reply_pkg );
            bool do_cmd_request( const msg_request* a_request, request_reply_package& a_reply_pkg );

            //bool send_reply( unsigned a_return_code, const std::string& a_return_msg, request_reply_package& a_pkg ) const;

            //param_node* create_sender_info() const;

            amqp_channel_ptr f_channel;
            std::string f_queue_name;
            std::string f_consumer_tag;

            run_server* f_run_server;
            config_manager* f_conf_mgr;
            acq_request_db* f_acq_request_db;
            server_worker* f_server_worker;

            atomic_bool f_canceled;

        private:
            //*****************
            // Request handlers
            //*****************

            bool handle_lock_request( const msg_request* a_request, request_reply_package& a_pkg );
            bool handle_unlock_request( const msg_request* a_request, request_reply_package& a_pkg );
            bool handle_is_locked_request( const msg_request* a_request, request_reply_package& a_pkg );

        public:
            /// enable lockout with randomly-generated key
            uuid_t enable_lockout( const param_node& a_tag );
            /// enable lockout with user-supplied key
            uuid_t enable_lockout( const param_node& a_tag, uuid_t a_key );
            bool disable_lockout( const uuid_t& a_key, bool a_force = false );

            bool is_locked() const;
            const param_node& get_lockout_tag() const;
            bool check_key( const uuid_t& a_key ) const;

        private:
            // Returns true if the server is unlocked or if it's locked and the key matches the lockout key; returns false otherwise.
            bool authenticate( const uuid_t& a_key ) const;

            param_node f_lockout_tag;
            uuid_t f_lockout_key;

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
