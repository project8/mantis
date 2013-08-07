#ifndef RECEIVER_HH_
#define RECEIVER_HH_

#include "callable.hh"

namespace mantis
{

    class server;
    class queue;

    class receiver :
        public callable
    {
        public:
            receiver( server* a_server, queue* a_queue );
            virtual ~receiver();

            void execute();

        private:
            server* f_server;
            queue* f_queue;
    };

}

#endif
