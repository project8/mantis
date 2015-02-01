#define MANTIS_API_EXPORTS

#include "mt_thread.hh"

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
#ifndef _WIN32
            pthread_create(&f_thread, 0, &thread::thread_setup_and_execute, this);
#else
            f_thread = CreateThread(NULL, 0, thread::thread_setup_and_execute, this, 0, &id);
#endif
            set_state(e_running);
        }
        return;
    }

    void thread::join()
    {
        if( get_state() == e_running )
        {
#ifndef _WIN32
            pthread_join(f_thread, 0);
#else
            WaitForSingleObject(f_thread, INFINITE);
#endif
        }
        return;
    }

    void thread::cancel()
    {
        if( get_state() == e_running )
        {
            f_object->cancel();
#ifndef _WIN32
            pthread_cancel(f_thread);
#else

#endif
            set_state(e_cancelled);
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

#ifndef _WIN32
    void* thread::thread_setup_and_execute(void* voidthread)
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

#else

    DWORD WINAPI thread::thread_setup_and_execute(PVOID voidthread)
    {
        thread* t_thread = (::mantis::thread*) (voidthread);
        callable* object = t_thread->f_object;
        object->execute();
        t_thread->set_state(e_complete);
        return 0;
    }
#endif

}
