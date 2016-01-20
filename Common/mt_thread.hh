#ifndef MT_THREAD_HH_
#define MT_THREAD_HH_

#include "mt_callable.hh"
#include "mt_mutex.hh"

#ifndef USE_CPP11
#include <pthread.h>
#else
#include <thread>
#endif

// Require C++11 to build in Windows
#ifndef USE_CPP11
#ifdef _WIN32
#error Windows build requires C++11
#endif
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
            void set_state( state a_state );

        private:
#ifndef USE_CPP11
            static void* thread_setup_and_execute(void* voidthread);
            static void thread_cleanup( void* voidthread );

            pthread_t f_thread;
#else
            void setup_and_execute();

            std::thread f_thread;
#endif

            mutex f_mutex;
            state f_state;
            callable* f_object;
    };
}

#endif
