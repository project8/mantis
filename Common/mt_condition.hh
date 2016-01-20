#ifndef MT_CONDITION_HH_
#define MT_CONDITION_HH_

#include "mt_api.hh"

#ifndef USE_CPP11
#include <pthread.h>
#include <cstddef>
#else
#include <condition_variable>
#include <mutex>
#endif

// Require C++11 to build in Windows
#ifndef USE_CPP11
#ifdef _WIN32
#error Windows build requires C++11
#endif
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
#ifndef USE_CPP11
            pthread_mutex_t f_mutex;
            pthread_cond_t f_condition;
#else
            std::mutex f_mutex;
            std::condition_variable f_condition;

            bool _continue_waiting();
#endif
    };


#ifdef USE_CPP11
    inline bool condition::_continue_waiting()
    {
        return ! is_waiting();
    }
#endif

}

#endif
