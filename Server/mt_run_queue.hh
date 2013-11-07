#ifndef MT_QUEUE_HH_
#define MT_QUEUE_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"
#include "mt_run_context.hh"

#include <list>

namespace mantis
{

    class run_queue :
        public callable
    {
        public:
            run_queue();
            virtual ~run_queue();

            bool empty();

            void to_front( run_context* a_run );
            run_context* from_front();

            void to_back( run_context* a_run );
            run_context* from_back();

            void execute();

        private:
            mutex f_mutex;
            std::list< run_context* > f_runs;
    };

}

#endif
