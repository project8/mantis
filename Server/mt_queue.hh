#ifndef MT_QUEUE_HH_
#define MT_QUEUE_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"
#include "mt_context.hh"

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
