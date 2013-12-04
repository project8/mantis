/*
 * mt_signal_handler.cc
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#include "mt_signal_handler.hh"

#include "mt_exception.hh"

#include <signal.h>

namespace mantis
{

    bool signal_handler::f_got_exit_signal = false;

    bool signal_handler::f_handling_sig_int = false;

    mutex signal_handler::f_mutex;
    signal_handler::threads signal_handler::f_threads;

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

    void signal_handler::push_thread( thread* a_thread )
    {
        f_mutex.lock();
        f_threads.push( a_thread );
        f_mutex.unlock();
        return;
    }

    void signal_handler::pop_thread()
    {
        f_mutex.lock();
        f_threads.pop();
        f_mutex.unlock();
        return;
    }

    void signal_handler::reset()
    {
        f_mutex.lock();
        f_got_exit_signal = false;
        f_handling_sig_int = false;
        while( ! f_threads.empty() )
        {
            f_threads.pop();
        }
        f_mutex.unlock();
        return;
    }

    bool signal_handler::got_exit_signal()
    {
        return f_got_exit_signal;
    }

    void signal_handler::handle_sig_int( int _ignored )
    {
        f_mutex.lock();
        f_got_exit_signal = true;
        while( ! f_threads.empty() )
        {
            f_threads.top()->cancel();
            f_threads.pop();
        }
        f_mutex.unlock();
        return;
    }

} /* namespace mantis */
