#ifndef MT_WORKER_HH_
#define MT_WORKER_HH_

#include "mt_callable.hh"

#include "mt_digitizer_px1500.hh"
#include "mt_writer.hh"
#include "mt_queue.hh"
#include "mt_condition.hh"

namespace mantis
{

    class worker :
        public callable
    {
        public:
            worker( digitizer_px1500* a_digitizer, writer* a_writer, queue* a_queue, condition* a_queue_condition, condition* a_buffer_condition );
            virtual ~worker();

            void execute();

        private:
            digitizer_px1500* f_digitizer;
            writer* f_writer;
            queue* f_queue;
            condition* f_queue_condition;
            condition* f_buffer_condition;
    };

}

#endif
