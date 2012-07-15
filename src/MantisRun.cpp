#include "MantisRun.hpp"

#include <unistd.h>

#include <iostream>
using std::cout;
using std::endl;

MantisRun::MantisRun() :
    fRunDuration( 0 )
{
}
MantisRun::~MantisRun()
{
}

MantisRun* MantisRun::runFromEnv( safeEnvPtr& env )
{
    MantisRun* NewRun = new MantisRun();

    NewRun->fRunDuration = (env.get())->getRunDuration();

    return NewRun;
}

void MantisRun::Initialize()
{
    fStatus->SetRunCondition( &fCondition );

    return;
}

void MantisRun::Execute()
{
    cout << "run will wait for <" << 1000000 * fRunDuration << "> nanoseconds" << endl;
    cout << "run sleeping..." << endl;
    sleep( 2 );
    cout << "run is removing blocks..." << endl;

    fStatus->SetRunning();

    fCondition.WaitFor( 1000000 * fRunDuration );

    if( !fStatus->IsError() )
    {
        fStatus->SetComplete();
    }
    return;
}

void MantisRun::Finalize()
{
    return;
}
