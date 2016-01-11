#ifndef MT_AMQP_RELAYER_HH_
#define MT_AMQP_RELAYER_HH_

#include "mt_callable.hh"

#include "mt_amqp.hh"
#include "mt_concurrent_queue.hh"
#include "mt_constants.hh"
#include "mt_message.hh"
#include "mt_mutex.hh"

#include <atomic>
#include <string>

namespace scarab
{
    class param_node;
}

namespace mantis
{
    using std::atomic_bool;

    class broker;

    struct request_reply_package;

    class MANTIS_API amqp_relayer : public callable
    {
        public:
            enum encoding
            {
                k_json
            };

            enum message_type
            {
                k_request = T_REQUEST,
                k_alert = T_ALERT
            };


        public:
            amqp_relayer( const broker* a_broker );
            virtual ~amqp_relayer();

            bool initialize( const scarab::param_node* a_amqp_config );

            void execute();

            //void stop_acquisition(); /// stops digitizing & writing; server worker continues to function

            void cancel(); /// cancels the relayer entirely

            //bool handle_stop_acq_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

        private:
            amqp_channel_ptr f_channel;

            std::string f_request_exchange;
            std::string f_alert_exchange;

            //boost::uuids::random_generator f_uuid_gen;
            /*
            struct message_data
            {
                const param_node* f_message;
                message_type f_message_type;
                std::string f_routing_key;
                encoding f_encoding;
            };
            */
            bool relay_request( message* a_message );
            bool relay_alert( message* a_message );

            //bool encode_message( const message_data* a_data, std::string& a_message ) const;

            concurrent_queue< message* > f_queue;

            atomic_bool f_canceled;

        public:
            bool send_message( message* a_message );

            //bool send_alert( const param_node* a_message, std::string a_routing_key, encoding a_encoding );

        private:
            //std::string interpret_encoding( encoding a_encoding) const;

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
            std::atomic< thread_state > f_digitizer_state, f_writer_state;

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
            std::atomic< status > f_status;
            */
    };

}

#endif
