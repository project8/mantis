#ifndef MT_THREAD_HH_
#define MT_THREAD_HH_

#include "mt_callable.hh"
#include "mt_mutex.hh"

#ifndef _WIN32
#include <pthread.h>
#endif

namespace mantis
{

    class MANTIS_API thread
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

            // thread-safe getter
            state get_state();
            // thread-safe setter
            void set_state( thread::state a_state );

        private:
#ifndef _WIN32
            static void* thread_setup_and_execute(void* voidthread);
            static void thread_cleanup( void* voidthread );

            pthread_t f_thread;
#else
            static DWORD WINAPI thread_setup_and_execute(PVOID voidthread);
            // windows does not have completely asyncronous cancellation

            HANDLE f_thread;
#endif

            mutex f_mutex;
            state f_state;
            callable* f_object;
    };
}

#endif
