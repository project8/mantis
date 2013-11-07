#ifndef MT_WORKER_HH_
#define MT_WORKER_HH_

#include "mt_callable.hh"

#include "mt_digitizer.hh"
#include "mt_writer.hh"
#include "mt_run_queue.hh"
#include "mt_condition.hh"

namespace mantis
{

    class worker :
        public callable
    {
        public:
            worker( digitizer* a_digitizer, writer* a_writer, run_context_queue* a_run_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~worker();

            void execute();

        private:
            digitizer* f_digitizer;
            writer* f_writer;
            run_context_queue* f_run_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;
    };

}

#endif
