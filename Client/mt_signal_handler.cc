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

    signal_handler::signal_handler() :
            f_threads()
    {
        if( signal( SIGINT, signal_handler::handle_sig_int ) == SIG_ERR )
        {
            throw exception << "Unable to handle SIGINT";
        }
    }

    signal_handler::~signal_handler()
    {
    }

    void signal_handler::add_thread( thread* a_thread )
    {
        f_threads.insert( a_thread );
        return;
    }

    void signal_handler::reset()
    {
        f_got_exit_signal = false;
        f_threads.clear();
        return;
    }

    void signal_handler::handle_sig_int( int _ignored )
    {
        f_got_exit_signal = true;
        for( set< thread* >::iterator it = f_threads.begin(); it != f_threads.end(); ++it )
        {
            (*it)->cancel();
        }
        return;
    }

} /* namespace Katydid */
