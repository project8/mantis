#ifndef MT_RUN_QUEUE_HH_
#define MT_RUN_QUEUE_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"

#include <list>

namespace mantis
{
    class run_context_dist;

    class run_queue : public callable
    {
        public:
            run_queue();
            virtual ~run_queue();

            bool empty();

            void to_front( run_context_dist* a_run );
            run_context_dist* from_front();

            void to_back( run_context_dist* a_run );
            run_context_dist* from_back();

            void execute();
            void cancel();

        private:
            mutex f_mutex;
            std::list< run_context_dist* > f_runs;
    };

}

#endif
