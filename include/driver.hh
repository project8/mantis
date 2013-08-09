#ifndef DRIVER_HH_
#define DRIVER_HH_

#include "callable.hh"

#include "queue.hh"
#include "digitizer.hh"
#include "writer.hh"

namespace mantis
{

    class queue;
    class digitizer;
    class writer;

    class driver :
        public callable
    {
        public:
            driver( queue* a_queue, digitizer* a_digitizer, writer* a_writer );
            virtual ~driver();

            void execute();

        private:
            queue* f_queue;
            digitizer* f_digitizer;
            writer* f_writer;
    };

}

#endif
