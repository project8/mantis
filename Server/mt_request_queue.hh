#ifndef MT_QUEUE_HH_
#define MT_QUEUE_HH_

#include "mt_callable.hh"

#include "mt_mutex.hh"
#include "mt_request_dist.hh"

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

            void to_front( request_dist* a_run );
            request_dist* from_front();

            void to_back( request_dist* a_run );
            request_dist* from_back();

            void execute();

        private:
            mutex f_mutex;
            std::list< request_dist* > f_runs;
    };

}

#endif
