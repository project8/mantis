#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_mutex.hh"

#include <string>

namespace mantis
{
    class buffer;
    class condition;
    class device_manager;
    class digitizer;
    class param_node;
    class run_context_dist;
    class acq_request_db;
    class run_queue;
    class thread;
    class writer;

    struct request_reply_package;

    class MANTIS_API server_worker : public callable
    {
        public:
            server_worker( device_manager* a_dev_mgr, acq_request_db* a_run_queue );
            virtual ~server_worker();

            void execute();

            void stop_acquisition(); /// stops digitizing & writing; server worker continues to function

            void cancel(); /// cancels the server worker entirely

            bool handle_stop_acq_request( const param_node& a_msg_payload, const std::string& a_mantis_routing_key, request_reply_package& a_pkg );

        private:
            device_manager* f_dev_mgr;
            acq_request_db* f_acq_request_db;

            // the server worker does not own the digitizer or writer
            // these pointers are here so that the worker can be cancelled by a different thread
            digitizer* f_digitizer;
            writer* f_writer;
            mutex f_component_mutex;

            atomic_bool f_canceled;

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
    };

    inline server_worker::thread_state server_worker::get_digitizer_state() const
    {
        return f_digitizer_state.load();
    }

    inline void server_worker::set_digitizer_state( thread_state a_thread_state )
    {
        f_digitizer_state.store( a_thread_state );
        return;
    }

    inline server_worker::thread_state server_worker::get_writer_state() const
    {
        return f_writer_state.load();
    }

    inline void server_worker::set_writer_state( thread_state a_thread_state )
    {
        f_writer_state.store( a_thread_state );
        return;
    }


    inline server_worker::status server_worker::get_status() const
    {
        return f_status.load();
    }

    inline void server_worker::set_status( status a_status )
    {
        f_status.store( a_status );
        return;
    }

}

#endif
