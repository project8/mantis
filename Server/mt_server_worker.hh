#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"

#include <string>

namespace mantis
{
    class buffer;
    class condition;
    class digitizer;
    class param_node;
    class run_context_dist;
    class run_database;
    class run_queue;
    class thread;
    class writer;

    class server_worker : public callable
    {
        public:
            server_worker( const param_node* a_config, digitizer* a_digitizer, buffer* a_buffer, run_database* a_run_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~server_worker();

            void execute();
            void cancel();

            void set_writer( writer* a_writer );

        private:
            const param_node* f_config;
            digitizer* f_digitizer;
            writer* f_writer;
            buffer* f_buffer;
            run_database* f_run_database;
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
