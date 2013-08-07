#include "thread.hh"

namespace mantis
{

    thread::thread( callable* an_object ) :
            f_thread(),
            f_state( e_ready ),
            f_object( an_object )
    {
    }

    thread::~thread()
    {
        if( f_state == e_running )
        {
            cancel();
        }
    }

    void thread::start()
    {
        if( f_state == e_ready )
        {
            pthread_create( &f_thread, 0, &thread::ThreadActionFunction, this );
            f_state = e_running;
        }
        return;
    }
    void thread::join()
    {
        if( f_state == e_running )
        {
            pthread_join( f_thread, 0 );
        }
        return;
    }
    void thread::cancel()
    {
        if( f_state == e_running )
        {
            pthread_cancel( f_thread );
            f_state = e_cancelled;
        }
        return;
    }
    void thread::reset()
    {
        f_state = e_ready;
        return;
    }

    const thread::state& thread::get_state()
    {
        return f_state;
    }

    void* thread::ThreadActionFunction( void* voidthread )
    {
        pthread_cleanup_push( &::mantis::thread::ThreadCleanupFunction, voidthread );
        thread* thread = (::mantis::thread*) (voidthread);
        callable* object = thread->f_object;
        object->execute();
        thread->f_state = e_complete;
        pthread_cleanup_pop( 0 );
        return 0;
    }

    void thread::ThreadCleanupFunction( void* voidthread )
    {
        thread* thread = (::mantis::thread*) (voidthread);
        thread->f_state = e_cancelled;
        return;
    }

}
