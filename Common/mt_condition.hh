#ifndef MT_CONDITION_HH_
#define MT_CONDITION_HH_

#include "mt_constants.hh"

#ifndef _WIN32
#include <pthread.h>
#include <cstddef>
#else
#include <Windows.h>
#endif

namespace mantis
{

    class MANTIS_API condition
    {
        public:
            condition();
            virtual ~condition();

            bool is_waiting();

            void wait();
            void release();

        private:
            bool f_state;
#ifndef _WIN32
            pthread_mutex_t f_mutex;
            pthread_cond_t f_condition;
#else
            CRITICAL_SECTION f_critical_section;
            CONDITION_VARIABLE f_condition;
#endif
    };

}

#endif
