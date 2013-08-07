#include "mutex.hh"

namespace mantis
{

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

}
