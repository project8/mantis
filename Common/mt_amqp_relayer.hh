#ifndef MT_AMQP_RELAYER_HH_
#define MT_AMQP_RELAYER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_concurrent_queue.hh"
#include "mt_constants.hh"
#include "mt_mutex.hh"

#include <boost/uuid/random_generator.hpp>

#include <string>

namespace mantis
{
    class broker;
    class param_node;

    struct request_reply_package;

    class MANTIS_API amqp_relayer : public callable
    {
        public:
            amqp_relayer( const param_node* a_amqp_config );
            virtual ~amqp_relayer();

            void execute();

            //void stop_acquisition(); /// stops digitizing & writing; server worker continues to function

            void cancel(); /// cancels the server worker entirely

            //bool handle_stop_acq_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

        private:
            broker* f_broker;

            std::string f_request_exchange;
            std::string f_alert_exchange;

            boost::uuids::random_generator f_uuid_gen;

            enum encoding
            {
                k_json
            };

            enum message_type
            {
                k_request = T_REQUEST,
                k_alert = T_ALERT
            };

            struct message_data
            {
                const param_node* f_message;
                message_type f_message_type;
                std::string f_routing_key;
                encoding f_encoding;
            };

            bool relay_request( message_data* a_data );
            bool relay_alert( message_data* a_data );

            bool encode_message( const message_data* a_data, std::string& a_message ) const;

            concurrent_queue< message_data* > f_queue;

            atomic_bool f_canceled;

        public:
            bool send_request( const param_node* a_message, std::string a_routing_key, encoding a_encoding );

            bool send_alert( const param_node* a_message, std::string a_routing_key, encoding a_encoding );

        private:
            std::string interpret_encoding( encoding a_encoding) const;

            /*

        public:
            enum thread_state
            {
                k_inactive,
                k_running
            };

            static std::string interpret_thread_state( thread_state a_thread_state );

            thread_state get_digitizer_state() const;
            void set_digitizer_state( thread_state a_thread_state );

            thread_state get_writer_state() const;
            void set_writer_state( thread_state a_thread_state );

        private:
            boost::atomic< thread_state > f_digitizer_state, f_writer_state;

        public:
            enum status
            {
                k_initialized = 0,
                k_starting = 1,
                k_idle = 4,
                k_acquiring = 5,
                k_acquired = 6,
                k_canceled = 9,
                k_error = 100
            };

            static std::string interpret_status( status a_status );

            status get_status() const;
            void set_status( status a_status );

        private:
            boost::atomic< status > f_status;
            */
    };

}

#endif
