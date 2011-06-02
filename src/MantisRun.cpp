#include "MantisRun.hpp"

#include <unistd.h>

#include <iostream>
using std::cout;
using std::endl;

MantisRun::MantisRun() :
    fStatus( NULL ), fDuration( 0 )
{
}
MantisRun::~MantisRun()
{
}

MantisRun* MantisRun::runFromEnv(safeEnvPtr& env, MantisStatus* sts)
{
  MantisRun* res = new MantisRun();
  res->SetDuration((env.get())->getRunLength());
  res->SetStatus(sts);

  return res;
}

void MantisRun::SetStatus( MantisStatus* aStatus )
{
    fStatus = aStatus;
    return;
}
void MantisRun::SetDuration( const unsigned int& aDuration )
{
    fDuration = aDuration;
    return;
}

void MantisRun::Initialize()
{
    return;
}

void MantisRun::Execute()
{
    fStatus->SetRunning();
    fStatus->GetWriterCondition()->Release();
    fStatus->GetReaderCondition()->Release();
    for( unsigned int Seconds = 1; Seconds <= fDuration; Seconds++ )
    {
        sleep( 1 );
        
        cout << ".";
        cout.flush();
        if( (Seconds % 10) == 0 )
        {
            cout << "\r";
            cout << "          ";
            cout << "\r";
            cout.flush();
        }
        
        if( fStatus->IsError() )
        {
            if( fStatus->GetWriterCondition()->IsWaiting() == true )
            {
                fStatus->GetWriterCondition()->Release(); 
            }
            if( fStatus->GetReaderCondition()->IsWaiting() == true )
            {
                fStatus->GetReaderCondition()->Release();
            }
            
            cout << "\nstopping on error." << endl;
            
            return;
        }
    }       
    fStatus->SetComplete();
    
    if( fStatus->GetWriterCondition()->IsWaiting() == true )
    {
        fStatus->GetWriterCondition()->Release(); 
    }
    if( fStatus->GetReaderCondition()->IsWaiting() == true )
    {
        fStatus->GetReaderCondition()->Release();
    }
    
    cout << "\nrun complete." << endl;
    
    return;
}

void MantisRun::Finalize()
{
    return;
}
