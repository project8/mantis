#ifndef MUTEX_HH_
#define MUTEX_HH_

#include <pthread.h>
#include <cstddef>

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
            pthread_mutex_t f_mutex;
    };

}

#endif
