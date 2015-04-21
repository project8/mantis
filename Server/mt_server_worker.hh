#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"

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

    class MANTIS_API server_worker : public callable
    {
        public:
            server_worker( device_manager* a_dev_mgr, acq_request_db* a_run_queue, condition* a_queue_condition );
            virtual ~server_worker();

            void execute();
            void cancel();

        private:
            device_manager* f_dev_mgr;
            acq_request_db* f_acq_request_db;
            condition* f_queue_condition;

            // the server worker does not own the digitizer or writer
            // these pointers are here so that the worker can be cancelled by a different thread
            digitizer* f_digitizer;
            writer* f_writer;

            atomic_bool f_canceled;

            enum thread_state
            {
                k_inactive,
                k_running
            } f_digitizer_state, f_writer_state;

    };
}

#endif
