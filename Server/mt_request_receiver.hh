#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_amqp.hh"
#include "mt_mutex.hh"
#include "mt_uuid.hh"

#include "hub.hh"

#include "param.hh"

#include "SimpleAmqpClient/Envelope.h"

#include <atomic>

using scarab::param_node;

namespace mantis
{
    using dripline::request_ptr_t;

    class acq_request_db;
    class buffer;
    class condition;
    class config_manager;
    class device_manager;
    class msg_request;
    class run_server;
    class server_worker;

    using std::atomic_bool;


    class MANTIS_API request_receiver : public callable, public dripline::hub
    {
        public:
            request_receiver( run_server* a_run_server, config_manager* a_conf_mgr, acq_request_db* a_acq_request_db, server_worker* a_server_worker );
            virtual ~request_receiver();

            void execute();
            void cancel();

        private:
            virtual bool do_run_request( const request_ptr_t a_request, reply_package& a_reply_pkg );
            virtual bool do_get_request( const request_ptr_t a_request, reply_package& a_reply_pkg );
            virtual bool do_set_request( const request_ptr_t a_request, reply_package& a_reply_pkg );
            virtual bool do_cmd_request( const request_ptr_t a_request, reply_package& a_reply_pkg );

            int f_listen_timeout_ms;

            run_server* f_run_server;
            config_manager* f_conf_mgr;
            acq_request_db* f_acq_request_db;
            server_worker* f_server_worker;

        public:
            enum status
            {
                k_initialized = 0,
                k_starting = 1,
                k_listening = 5,
                k_canceled = 9,
                k_done = 10,
                k_error = 100
            };

            static std::string interpret_status( status a_status );

            status get_status() const;
            void set_status( status a_status );

        private:
            std::atomic< status > f_status;

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
