#ifndef QUEUE_HH_
#define QUEUE_HH_

#include "callable.hh"

#include "mutex.hh"
#include "context.hh"

#include <list>

namespace mantis
{

    class queue :
        public callable
    {
        public:
            queue();
            virtual ~queue();

            bool empty();

            void to_front( context* a_run );
            context* from_front();

            void to_back( context* a_run );
            context* from_back();

            void execute();

        private:
            mutex f_mutex;
            std::list< context* > f_runs;
    };

}

#endif
