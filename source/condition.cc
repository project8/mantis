#include "condition.hh"

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
        bool StateCopy;
        pthread_mutex_lock( &f_mutex );
        StateCopy = f_state;
        pthread_mutex_unlock( &f_mutex );
        return StateCopy;
    }

    void condition::wait()
    {
        pthread_mutex_lock( &f_mutex );
        f_state = true;
        while( f_state == true )
        {
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
