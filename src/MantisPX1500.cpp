#include "MantisPX1500.hpp"

#include <cstdlib>
#include <unistd.h>

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

MantisPX1500::MantisPX1500() :
     fCondition(), fHandle(), fAcquisitionCount( 0 ), fRecordCount( 0 ), fDeadTickCount( 0 ), fStatus( NULL ), fBuffer( NULL ), fDigitizationRate( 0. )
{
}
MantisPX1500::~MantisPX1500()
{
}

MantisPX1500* MantisPX1500::digFromEnv(safeEnvPtr& env, 
				       MantisStatus* sts,
				       MantisBuffer* buf)
{
  MantisPX1500* res = new MantisPX1500();
  res->SetDigitizationRate((env.get())->getClockRate());
  res->SetStatus(sts);
  res->SetBuffer(buf);

  return res;
}

void MantisPX1500::SetStatus( MantisStatus* aStatus )
{
    fStatus = aStatus;
    return;
}
void MantisPX1500::SetBuffer( MantisBuffer* aBuffer )
{
    fBuffer = aBuffer;
    return;
}
void MantisPX1500::SetDigitizationRate( const double& aRate )
{
    fDigitizationRate = aRate;
    return;
}

void MantisPX1500::Initialize()
{
    fStatus->SetWriterCondition( &fCondition );
    
    int PX4Result;
    
    PX4Result = ConnectToDevicePX4( &fHandle, 1 );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to connect to digitizer card: " );
        exit(-1);
    }

    PX4Result = SetPowerupDefaultsPX4( fHandle );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to enter default state: " );
        exit(-1);
    }

    PX4Result = SetActiveChannelsPX4( fHandle, PX4CHANSEL_SINGLE_CH1 );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to activate channel 1: " );
        exit(-1);
    }

    PX4Result = SetInternalAdcClockRatePX4( fHandle, fDigitizationRate );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to set sampling rate: " );
        exit(-1);
    }
    
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    for( size_t Count = 0; Count < fBuffer->GetBufferLength(); Count++ )
    {
        PX4Result = AllocateDmaBufferPX4( fHandle, fBuffer->GetDataLength(), &Iterator->Data()->fDataPtr );
        if( PX4Result != SIG_SUCCESS )
        {
            stringstream Converter;
            Converter << "failed to allocate DMA buffer " << Count << ": " ;
            DumpLibErrorPX4( PX4Result, Converter.str().c_str() );
            exit(-1);
        }
        Iterator->Increment();
    }
    delete Iterator;
    
    return;
}

void MantisPX1500::Execute()
{   
    //allocate some local variables
    int PX4Result;
    clock_t ReferenceTick;
    ReferenceTick = clock();
    
    clock_t DeadStartTick;
    clock_t DeadEndTick;
    
    //grab an iterator
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    
    //wait for run to release me
    fCondition.Wait();
    if( fStatus->IsRunning() == false )
    {
        delete Iterator;
        return;
    }
    
    //start acquisition
    PX4Result = BeginBufferedPciAcquisitionPX4( fHandle, PX4_FREE_RUN );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to begin dma acquisition: " );
        fStatus->SetError();
        return;
    }
    fAcquisitionCount++;
    
    //go go go go
    while( true )
    {
        if( fStatus->IsRunning() == false )
        {
            EndBufferedPciAcquisitionPX4( fHandle );
            delete Iterator;
            return;
        }
        
        Iterator->SetWriting();
        Iterator->Data()->fId = fAcquisitionCount;
        Iterator->Data()->fTick = clock();
        PX4Result = GetPciAcquisitionDataFastPX4( fHandle, fBuffer->GetDataLength(), Iterator->Data()->fDataPtr, 0 );
        if( PX4Result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( PX4Result, "failed to acquire dma data over pci: " );
            fStatus->SetError();
            PX4Result = EndBufferedPciAcquisitionPX4( fHandle );
            delete Iterator;
            return;
        }
        fRecordCount++;
        Iterator->SetWritten();
        
        if( Iterator->TryIncrement() == false )
        {
            PX4Result = EndBufferedPciAcquisitionPX4( fHandle );

            DeadStartTick = clock();
            cout << "blocked\n";
            fCondition.Wait();
            cout << "released\n";
            DeadEndTick = clock();
            fDeadTickCount += (DeadEndTick - DeadStartTick);
            
            if( fStatus->IsRunning() == false )
            {
                delete Iterator;
                return;
            }

            PX4Result = BeginBufferedPciAcquisitionPX4( fHandle, PX4_FREE_RUN );
            fAcquisitionCount++;
            
            Iterator->Increment();
        }              
    }
    return;
}  

void MantisPX1500::Finalize()
{
    int PX4Result;
    
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    for( size_t Count = 0; Count < fBuffer->GetBufferLength(); Count++ )
    {
        PX4Result = FreeDmaBufferPX4( fHandle, Iterator->Data()->fDataPtr );
        if( PX4Result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( PX4Result, "failed to deallocate DMA buffer: " );
            exit(-1);
        }
        Iterator->Increment();
    }
    delete Iterator;
    
    PX4Result = DisconnectFromDevicePX4( fHandle );
    if( PX4Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( PX4Result, "failed to disconnect from digitizer card: " );
        exit(-1);
    }
    
    cout << "aquisitions taken: " << fAcquisitionCount << endl;
    cout << "records taken: " << fRecordCount << endl;
    cout << "dead time: " << (double)(fDeadTickCount) / (double)(CLOCKS_PER_SEC) << endl;

    return;
}
