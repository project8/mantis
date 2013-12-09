/*
 * mt_signal_handler.cc
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#include "mt_signal_handler.hh"

#include "mt_exception.hh"

#include <iostream>
#include <signal.h>

namespace mantis
{

    bool signal_handler::f_got_exit_signal = false;

    bool signal_handler::f_handling_sig_int = false;
    bool signal_handler::f_handling_sig_quit = false;

    mutex signal_handler::f_mutex;
    signal_handler::threads signal_handler::f_threads;

    signal_handler::signal_handler()
    {
        if( ! f_handling_sig_int && signal( SIGINT, signal_handler::handler_cancel_threads ) == SIG_ERR )
        {
            throw exception() << "Unable to handle SIGINT\n";
        }
        else
        {
            f_handling_sig_int = true;
        }

        if( ! f_handling_sig_quit && signal( SIGQUIT, signal_handler::handler_cancel_threads ) == SIG_ERR )
        {
            throw exception() << "Unable to handle SIGQUIT\n";
        }
        else
        {
            f_handling_sig_quit = true;
        }

        if( signal(SIGPIPE, SIG_IGN) == SIG_ERR )
        {
            throw exception() << "Unable to ignore SIGPIPE\n";
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
        f_handling_sig_quit = false;
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

    void signal_handler::handler_cancel_threads( int _ignored )
    {
        print_message();

        f_mutex.lock();
        f_got_exit_signal = true;
        while( ! f_threads.empty() )
        {
            f_threads.top()->cancel();
            f_threads.pop();
            usleep( 100 );
        }
        f_mutex.unlock();
        return;
    }

    void signal_handler::print_message()
    {
        std::cout << "Hello!  Your signal is being handled by signal_handler.\n";
        std::cout << "Have a nice day!" << std::endl;
        return;
    }

} /* namespace mantis */
