#ifndef MT_THREAD_HH_
#define MT_THREAD_HH_

#include "mt_callable.hh"

#include <pthread.h>

namespace mantis
{

    class thread
    {
        public:
            typedef enum{ e_ready, e_running, e_cancelled, e_complete } state;

        public:
            thread( callable* an_object );
            virtual ~thread();

            void start();
            void join();
            void cancel();
            void reset();

            const state& get_state();

        private:
            static void* ThreadActionFunction( void* voidthread );
            static void ThreadCleanupFunction( void* voidstate );

            pthread_t f_thread;
            state f_state;
            callable* f_object;
    };

}

#endif
