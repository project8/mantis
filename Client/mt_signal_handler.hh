/*
 * mt_signal_handler.hh
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#ifndef MT_SIGNAL_HANDLER_HH_
#define MT_SIGNAL_HANDLER_HH_

#include "mt_thread.hh"

#include <set>

namespace mantis
{

    class signal_handler
    {
        public:
            typedef std::set< thread* > thread_set;

        public:
            signal_handler();
            virtual ~signal_handler();

            void add_thread( thread* );

            void reset();

            static void handle_sig_int( int _ignored );

        private:
            static thread_set f_threads;

            static bool f_got_exit_signal;

            static bool f_handling_sig_int;

    };

} /* namespace Katydid */
#endif /* MT_SIGNAL_HANDLER_HH_ */
