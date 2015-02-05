#ifndef MT_MUTEX_HH_
#define MT_MUTEX_HH_

#include "mt_constants.hh"

#ifndef _WIN32
#include <pthread.h>
#include <cstddef>
#else
#include <Windows.h>
#endif

namespace mantis
{

    class MANTIS_API mutex
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
