#include "MantisRun.hpp"

#include <unistd.h>

#include <iostream>
using std::cout;
using std::endl;

MantisRun::MantisRun() :
    fStatus( NULL ),
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
    cout << "initializing run with status pointer <" << fStatus << ">" << endl;
    fStatus->SetRunCondition( &fCondition );

    return;
}

void MantisRun::Execute()
{
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
