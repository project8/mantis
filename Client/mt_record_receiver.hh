#ifndef MT_RECEIVER_HH_
#define MT_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_server.hh"
#include "mt_request_queue.hh"
#include "mt_condition.hh"

namespace mantis
{

    class receiver :
        public callable
    {
        public:
            receiver( server* a_server, request_queue* a_request_queue, condition* a_condition );
            virtual ~receiver();

            void execute();

        private:
            server* f_server;
            request_queue* f_request_queue;
            condition* f_condition;
    };

}

#endif
