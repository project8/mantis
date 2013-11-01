#ifndef WORKER_HH_
#define WORKER_HH_

#include "callable.hh"

#include "digitizer.hh"
#include "writer.hh"
#include "queue.hh"
#include "condition.hh"

namespace mantis
{

    class worker :
        public callable
    {
        public:
            worker( digitizer* a_digitizer, writer* a_writer, queue* a_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~worker();

            void execute();

        private:
            digitizer* f_digitizer;
            writer* f_writer;
            queue* f_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;
    };

}

#endif
