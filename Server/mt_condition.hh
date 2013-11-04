#ifndef MT_CONDITION_HH_
#define MT_CONDITION_HH_

#include <pthread.h>
#include <cstddef>

namespace mantis
{

    class condition
    {
        public:
            condition();
            virtual ~condition();

            bool is_waiting();

            void wait();
            void release();

        private:
            bool f_state;
            pthread_mutex_t f_mutex;
            pthread_cond_t f_condition;
    };

}

#endif
