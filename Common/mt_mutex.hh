#ifndef MT_MUTEX_HH_
#define MT_MUTEX_HH_

#ifndef _WIN32
#include <pthread.h>
#include <cstddef>
#else
#include <Windows.h>
#endif

namespace mantis
{

    class mutex
    {
        public:
            mutex();
            virtual ~mutex();

            bool trylock();

            void lock();
            void unlock();

        private:
#ifndef _WIN32
            pthread_mutex_t f_mutex;
#else
            CRITICAL_SECTION f_critical_section;
#endif
    };

}

#endif
