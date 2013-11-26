#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

namespace mantis
{
    class configurator;
    class digitizer;
    class buffer;
    class request_queue;
    class condition;
    class writer;

    class server_worker :
        public callable
    {
        public:
            server_worker( configurator* a_config, digitizer* a_digitizer, buffer* a_buffer, request_queue* a_request_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~server_worker();

            void execute();

            void set_writer( writer* a_writer );

        private:
            configurator* f_config;
            digitizer* f_digitizer;
            writer* f_writer;
            buffer* f_buffer;
            request_queue* f_request_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;
    };
}

#endif
