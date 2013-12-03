/*
 * mt_signal_handler.cc
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#include "mt_signal_handler.hh"

#include "mt_exception.hh"

using std::set;

namespace mantis
{

    bool signal_handler::f_got_exit_signal = false;

    bool signal_handler::f_handling_sig_int = false;

    mutex signal_handler::f_mutex;
    signal_handler::thread_set signal_handler::f_threads;

    signal_handler::signal_handler()
    {
        if( ! f_handling_sig_int && signal( SIGINT, signal_handler::handle_sig_int ) == SIG_ERR )
        {
            throw exception() << "Unable to handle SIGINT\n";
        }
        else
        {
            f_handling_sig_int = true;
        }
    }

    signal_handler::~signal_handler()
    {
    }

    void signal_handler::add_thread( thread* a_thread )
    {
        f_mutex.lock();
        f_threads.insert( a_thread );
        f_mutex.unlock();
        return;
    }

    void signal_handler::reset()
    {
        f_mutex.lock();
        f_got_exit_signal = false;
        f_handling_sig_int = false;
        f_threads.clear();
        f_mutex.unlock();
        return;
    }

    void signal_handler::handle_sig_int( int _ignored )
    {
        f_mutex.lock();
        f_got_exit_signal = true;
        for( set< thread* >::iterator it = f_threads.begin(); it != f_threads.end(); ++it )
        {
            (*it)->cancel();
        }
        f_mutex.unlock();
        return;
    }

} /* namespace Katydid */
