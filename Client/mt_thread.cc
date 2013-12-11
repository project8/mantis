#include "mt_thread.hh"

#include <iostream>
using std::cout;
using std::endl;

namespace mantis
{

    thread::thread( callable* an_object ) :
            f_mutex(),
            f_thread(),
            f_state( e_ready ),
            f_object( an_object )
    {
    }

    thread::~thread()
    {
        if( get_state() == e_running )
        {
            cancel();
        }
    }

    void thread::start()
    {
        if( get_state() == e_ready )
        {
            pthread_create( &f_thread, 0, &thread::thread_setup_and_execute, this );
            set_state( e_running );
        }
        return;
    }
    void thread::join()
    {
        if( get_state() == e_running )
        {
            pthread_join( f_thread, 0 );
        }
        return;
    }
    void thread::cancel()
    {
        if( get_state() == e_running )
        {
            f_object->cancel();
            pthread_cancel( f_thread );
            set_state( e_cancelled );
        }
        return;
    }
    void thread::reset()
    {
        if( get_state() == e_running )
        {
            cancel();
        }
        set_state( e_ready );
        return;
    }

    thread::state thread::get_state()
    {
        thread::state t_state;
        f_mutex.lock();
        t_state = f_state;
        f_mutex.unlock();
        return t_state;
    }

    void thread::set_state( thread::state a_state )
    {
        f_mutex.lock();
        f_state = a_state;
        f_mutex.unlock();
        return;
    }

    void* thread::thread_setup_and_execute( void* voidthread )
    {
        pthread_cleanup_push( &::mantis::thread::thread_cleanup, voidthread );
        thread* t_thread = (::mantis::thread*) (voidthread);
        callable* object = t_thread->f_object;
        object->execute();
        t_thread->set_state( e_complete );
        pthread_cleanup_pop( 0 );
        pthread_exit( 0 );
    }

    void thread::thread_cleanup( void* /*voidthread*/ )
    {
        return;
    }

}
