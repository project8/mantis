#define MANTIS_API_EXPORTS

#include "mt_condition.hh"

namespace mantis
{
#ifndef _WIN32

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
        //pthread_cond_signal( &f_condition );
        pthread_cond_broadcast( &f_condition );
        pthread_mutex_unlock( &f_mutex );
        return;
    }

#else /* _WIN32 */

    condition::condition() :
        f_state(false)
    {
        InitializeCriticalSection(&f_critical_section);
        InitializeConditionVariable(&f_condition);
    }

    condition::~condition()
    {
        // there doesn't seem to be a function for deleting a condition variable
        DeleteCriticalSection(&f_critical_section);
    }

    bool condition::is_waiting()
    {
        bool t_state;
        EnterCriticalSection(&f_critical_section);
        t_state = f_state;
        LeaveCriticalSection(&f_critical_section);
        return t_state;
    }

    void condition::wait()
    {
        EnterCriticalSection(&f_critical_section);
        f_state = true;
        while (f_state == true)
        {
            // unlocks the mutex and waits for signal (via condition::release) from a different thread
            SleepConditionVariableCS(&f_condition, &f_critical_section, INFINITE);
        }
        LeaveCriticalSection(&f_critical_section);
        return;
    }

    void condition::release()
    {
        EnterCriticalSection(&f_critical_section);
        f_state = false;
        WakeConditionVariable(&f_condition);
        LeaveCriticalSection(&f_critical_section);
        return;
    }

#endif /* _WIN32 */

}
