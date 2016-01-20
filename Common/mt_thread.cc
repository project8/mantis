#define MANTIS_API_EXPORTS

#include "mt_thread.hh"

namespace mantis
{

    thread::thread( callable* an_object ) :
            f_thread(),
            f_mutex(),
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
#ifndef USE_CPP11
            pthread_create(&f_thread, 0, &thread::thread_setup_and_execute, this);
#else
            f_thread = std::thread( [this]{ setup_and_execute(); } );
#endif
            set_state( e_running );
        }
        return;
    }

    void thread::join()
    {
        if( get_state() == e_running )
        {
#ifndef USE_CPP11
            pthread_join(f_thread, 0);
#else
            f_thread.join();
#endif
        }
        return;
    }

    void thread::cancel()
    {
        if( get_state() == e_running )
        {
            f_object->cancel();
#ifndef USE_CPP11
            pthread_cancel(f_thread);
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

#ifndef USE_CPP11

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

    void thread::setup_and_execute()
    {
        f_object->execute();
        set_state( e_complete );
    }

#endif

}
