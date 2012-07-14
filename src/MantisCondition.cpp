#include "MantisCondition.hpp"

#include <iostream>
using std::cout;
using std::endl;

MantisCondition::MantisCondition() :
    fState( false ), fMutex(), fCondition()
{
    pthread_mutex_init( &fMutex, NULL );
    pthread_cond_init( &fCondition, NULL );
}
MantisCondition::~MantisCondition()
{
    pthread_cond_destroy( &fCondition );
    pthread_mutex_destroy( &fMutex );
}

bool MantisCondition::IsWaiting()
{
    bool StateCopy;
    pthread_mutex_lock( &fMutex );
    StateCopy = fState;
    pthread_mutex_unlock( &fMutex );
    return StateCopy;
};

void MantisCondition::Wait()
{
    pthread_mutex_lock( &fMutex );
    fState = true;
    while( fState == true )
    {
        pthread_cond_wait( &fCondition, &fMutex );
    }
    pthread_mutex_unlock( &fMutex );
    return;
}
void MantisCondition::WaitFor( const unsigned int& tNanoseconds)
{
    static const long sOneBillion = 1000000000;

    pthread_mutex_lock( &fMutex );
    fState = true;
    while( fState == true )
    {
        timespec tTime;
        clock_gettime( CLOCK_REALTIME, &tTime );
        tTime.tv_nsec += tNanoseconds;
        if( tTime.tv_nsec > sOneBillion )
        {
            long tRemainder = tTime.tv_nsec % sOneBillion;
            long tSeconds = (tTime.tv_nsec - tRemainder) / sOneBillion;
            tTime.tv_sec += tSeconds;
            tTime.tv_nsec = tRemainder;
        }
        pthread_cond_timedwait( &fCondition, &fMutex, &tTime );
    }
    pthread_mutex_unlock( &fMutex );
    return;
}
void MantisCondition::Release()
{
    pthread_mutex_lock( &fMutex );
    fState = false;
    pthread_cond_signal( &fCondition );
    pthread_mutex_unlock( &fMutex );
    return;
}
