/*
 * mt_signal_handler.cc
 *
 *  Created on: Dec 3, 2013
 *      Author: nsoblath
 */

#define MANTIS_API_EXPORTS

#include "mt_signal_handler.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"

#include <signal.h>
#ifndef _WIN32
#include <unistd.h> //usleep
#else
#include <Windows.h> //Sleep
#endif

namespace mantis
{
    MTLOGGER( mtlog, "signal_handler" );

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

#ifndef _WIN32
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
#endif
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

    void signal_handler::handler_cancel_threads( int )
    {
        print_message();

        f_mutex.lock();
        f_got_exit_signal = true;
        while( ! f_threads.empty() )
        {
            f_threads.top()->cancel();
            f_threads.pop();
#ifndef _WIN32
            usleep( 100 );
#else
            Sleep( 1 );
#endif
        }
        f_mutex.unlock();

#ifdef _WIN32
        ExitProcess( 1 );
#endif
        return;
    }

    void signal_handler::print_message()
    {
        MTINFO( mtlog, "\n\nHello!  Your signal is being handled by signal_handler.\n"
             << "Have a nice day!\n" );
        return;
    }

} /* namespace mantis */
