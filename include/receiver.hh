#ifndef RECEIVER_HH_
#define RECEIVER_HH_

#include "callable.hh"

#include "server.hh"
#include "queue.hh"
#include "condition.hh"

namespace mantis
{

    class receiver :
        public callable
    {
        public:
            receiver( server* a_server, queue* a_queue, condition* a_condition );
            virtual ~receiver();

            void execute();

        private:
            server* f_server;
            queue* f_queue;
            condition* f_condition;
    };

}

#endif
