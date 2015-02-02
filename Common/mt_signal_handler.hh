/*
 * mt_signal_handler.hh
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#ifndef MT_SIGNAL_HANDLER_HH_
#define MT_SIGNAL_HANDLER_HH_

#include "mt_mutex.hh"
#include "mt_thread.hh"

#include <stack>


namespace mantis
{
#ifdef _WIN32
    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::deque< thread*, std::allocator< thread* > >;
    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API std::stack< thread*, std::deque< thread*, std::allocator< thread* > > >;
#endif

    class MANTIS_API signal_handler
    {
        public:
            typedef std::stack< thread* > threads;

        public:
            signal_handler();
            virtual ~signal_handler();

            void push_thread( thread* );
            void pop_thread();

            void reset();

            static bool got_exit_signal();

            static void handler_cancel_threads( int _ignored );

        private:
            static void print_message();

            static mutex f_mutex;
            static threads f_threads;

            static bool f_got_exit_signal;

            static bool f_handling_sig_quit;
            static bool f_handling_sig_int;

    };

} /* namespace mantis */
#endif /* MT_SIGNAL_HANDLER_HH_ */
