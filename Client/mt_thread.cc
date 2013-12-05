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
        //cout << "in thread::start; state is: " << f_state << endl;
        if( get_state() == e_ready )
        {
            pthread_create( &f_thread, 0, &thread::thread_setup_and_execute, this );
            set_state( e_running );
            //cout << "thread::start changed state to: " << f_state << endl;
        }
        return;
    }
    void thread::join()
    {
        if( get_state() == e_running )
        {
            pthread_join( f_thread, 0 );
        }
        cout << "returning from join" << endl;
        return;
    }
    void thread::cancel()
    {
        //cout << "in thread::cancel; state is: " << f_state << endl;
        if( get_state() == e_running )
        {
            //cout << "thread::cancel is calling pthread_cancel" << endl;
            f_object->cancel();
            pthread_cancel( f_thread );
            set_state( e_cancelled );
            //cout << "thread::cancel changed state to: " << f_state << endl;
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
        //cout << "in setup and execute" << endl;
        thread* t_thread = (::mantis::thread*) (voidthread);
        callable* object = t_thread->f_object;
        object->execute();
        //cout << "completing thread" << endl;
        t_thread->set_state( e_complete );
        pthread_cleanup_pop( 0 );
        pthread_exit( 0 );
    }

    void thread::thread_cleanup( void* voidthread )
    {
        //cout << "in thread_cleanup" << endl;
        //thread* t_thread = (::mantis::thread*) (voidthread);
        //cout << "executing cleanup function" << endl;
        //t_thread->f_object->cancel();
        //t_thread->set_state( e_cancelled );
        return;
    }

}
