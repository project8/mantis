#ifndef WORKER_HH_
#define WORKER_HH_

#include "callable.hh"

namespace mantis
{

    class worker :
        public callable
    {
        public:
            worker( run* a_run, digitizer* a_digitizer, writer* a_writer );
            virtual ~worker();

            void execute();

        private:
            run* f_run;
            digitizer* f_digitizer;
            writer* f_writer;
    };

}

#endif
