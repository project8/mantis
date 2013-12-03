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
            static void* thread_setup_and_execute( void* voidthread );
            static void thread_cleanup( void* voidthread );

            pthread_t f_thread;
            state f_state;
            callable* f_object;
    };
}

#endif
