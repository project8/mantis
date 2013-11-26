#ifndef MT_QUEUE_HH_
#define MT_QUEUE_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"
#include "mt_run_context_dist.hh"

#include <list>

namespace mantis
{

    class request_queue :
        public callable
    {
        public:
            request_queue();
            virtual ~request_queue();

            bool empty();

            void to_front( run_context_dist* a_run );
            run_context_dist* from_front();

            void to_back( run_context_dist* a_run );
            run_context_dist* from_back();

            void execute();

        private:
            mutex f_mutex;
            std::list< run_context_dist* > f_runs;
    };

}

#endif
