#include "mt_mutex.hh"

namespace mantis
{
#ifndef _WIN32

    mutex::mutex()
    {
        pthread_mutex_init( &f_mutex, NULL );
    }
    mutex::~mutex()
    {
        pthread_mutex_destroy( &f_mutex );
    }

    bool mutex::trylock()
    {
        if( pthread_mutex_trylock( &f_mutex ) == 0 )
        {
            return true;
        }
        return false;
    }

    void mutex::lock()
    {
        pthread_mutex_lock( &f_mutex );
        return;
    }
    void mutex::unlock()
    {
        pthread_mutex_unlock( &f_mutex );
        return;
    }

#else /* _WIN32 */

    mutex::mutex()
    {
        InitializeCriticalSection(&f_critical_section);
    }
    mutex::~mutex()
    {
        DeleteCriticalSection(&f_critical_section);
    }

    bool mutex::trylock()
    {
        return TryEnterCriticalSection(&f_critical_section);
    }

    void mutex::lock()
    {
        EnterCriticalSection(&f_critical_section);
        return;
    }
    void mutex::unlock()
    {
        LeaveCriticalSection(&f_critical_section);
        return;
    }

#endif /* _WIN32 */
}
