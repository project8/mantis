#ifndef QUEUE_HH_
#define QUEUE_HH_

#include "mutex.hh"
#include "run.hh"

#include <list>

namespace mantis
{

    class queue
    {
        public:
            queue();
            virtual ~queue();

            bool is_empty();

            void to_front( run* a_run );
            run* from_front();

            void to_back( run* a_run );
            run* from_back();

            void for_each( void (run::* a_command )() );

        private:
            mutex f_mutex;
            std::list< run* > f_runs;
    };

}

#endif
