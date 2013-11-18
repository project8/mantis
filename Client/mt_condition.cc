#include "mt_condition.hh"

namespace mantis
{

    condition::condition() :
        f_state( false ),
        f_mutex(),
        f_condition()
    {
        pthread_mutex_init( &f_mutex, NULL );
        pthread_cond_init( &f_condition, NULL );
    }
    condition::~condition()
    {
        pthread_cond_destroy( &f_condition );
        pthread_mutex_destroy( &f_mutex );
    }

    bool condition::is_waiting()
    {
        bool t_state;
        pthread_mutex_lock( &f_mutex );
        t_state = f_state;
        pthread_mutex_unlock( &f_mutex );
        return t_state;
    }

    void condition::wait()
    {
        pthread_mutex_lock( &f_mutex );
        f_state = true;
        while( f_state == true )
        {
            // unlocks the mutex and waits for signal (via condition::release) from a different thread
            pthread_cond_wait( &f_condition, &f_mutex );
        }
        pthread_mutex_unlock( &f_mutex );
        return;
    }
    void condition::release()
    {
        pthread_mutex_lock( &f_mutex );
        f_state = false;
        pthread_cond_signal( &f_condition );
        pthread_mutex_unlock( &f_mutex );
        return;
    }

}
