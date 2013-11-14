#ifndef MT_SERVER_WORKER_HH_
#define MT_SERVER_WORKER_HH_

#include "mt_callable.hh"

#include "mt_digitizer.hh"
#include "mt_writer.hh"
#include "mt_request_queue.hh"
#include "mt_condition.hh"

namespace mantis
{

    class server_worker :
        public callable
    {
        public:
            server_worker( digitizer* a_digitizer, writer* a_writer, request_queue* a_request_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~server_worker();

            void execute();

        private:
            digitizer* f_digitizer;
            writer* f_writer;
            request_queue* f_request_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;
    };

}

#endif
