#ifndef MT_RECEIVER_HH_
#define MT_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_server.hh"
#include "mt_queue.hh"
#include "mt_condition.hh"

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