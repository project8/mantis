#include "MantisCondition.hpp"

#include "MantisTime.hpp"

#include <iostream>
using std::cout;
using std::endl;

MantisCondition::MantisCondition() :
    fState( false ),
    fMutex(),
    fCondition()
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
}

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
void MantisCondition::WaitFor( long tDelayNanoseconds )
{
    static const long sOneBillion = 1000000000;

    pthread_mutex_lock( &fMutex );
    fState = true;

    timespec tTime;
    MantisTimeGetMonotonic( &tTime );

    cout << "told to wait for <" << tDelayNanoseconds << "> nanoseconds" << endl;
    cout << "old seconds: <" << tTime.tv_sec << ">";
    cout << "old nanoseconds: <" << tTime.tv_nsec << ">";

    long tNewSeconds = tTime.tv_sec;
    long tNewNanoseconds = tTime.tv_nsec + tDelayNanoseconds;
    if( tNewNanoseconds > sOneBillion )
    {
        tNewSeconds += (tNewNanoseconds - tNewNanoseconds % sOneBillion) / sOneBillion;
        tNewNanoseconds = tNewNanoseconds % sOneBillion;
    }
    tTime.tv_sec = tNewSeconds;
    tTime.tv_nsec = tNewNanoseconds;

    cout << "new seconds: <" << tTime.tv_sec << ">";
    cout << "new nanoseconds: <" << tTime.tv_nsec << ">";

    pthread_cond_timedwait( &fCondition, &fMutex, &tTime );

    if( fState == true )
    {
        fState = false;
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
