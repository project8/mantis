#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"

namespace mantis
{
    class buffer;
    class condition;
    class configurator;
    class digitizer;
    class run_context_dist;
    class run_queue;
    class thread;
    class writer;

    class server_worker : public callable
    {
        public:
            server_worker( configurator* a_config, digitizer* a_digitizer, buffer* a_buffer, run_queue* a_run_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~server_worker();

            void execute();
            void cancel();

            void set_writer( writer* a_writer );

        private:
            configurator* f_config;
            digitizer* f_digitizer;
            writer* f_writer;
            buffer* f_buffer;
            run_queue* f_run_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;

            atomic_bool f_canceled;

            enum thread_state
            {
                k_inactive,
                k_running
            } f_digitizer_state, f_writer_state;

    };
}

#endif
