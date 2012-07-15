#include "MantisPX1500.hpp"

#include <sys/time.h>
#include <cstdlib>

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

MantisPX1500::MantisPX1500() :
    fHandle(),
    fAcquisitionCount( 0 ),
    fRecordCount( 0 ),
    fLiveMicroseconds( 0 ),
    fDeadMicroseconds( 0 ),
    fDigitizationRate( 0. ),
    fChannelMode( 1 ),
    fRecordLength( 0 ),
    fBufferCount( 0 )
{
}
MantisPX1500::~MantisPX1500()
{
}

MantisPX1500* MantisPX1500::digFromEnv( safeEnvPtr& env )
{
    MantisPX1500* NewPX1500 = new MantisPX1500();

    NewPX1500->fDigitizationRate = (env.get())->getAcquisitionRate();
    NewPX1500->fChannelMode = (env.get())->getChannelMode();
    NewPX1500->fRecordLength = (env.get())->getRecordLength();
    NewPX1500->fBufferCount = (env.get())->getBufferCount();

    return NewPX1500;
}

void MantisPX1500::Initialize()
{
    fStatus->SetPX1500Condition( &fCondition );

    int tResult;

    cout << "  *connecting to digitizer card..." << endl;

    tResult = ConnectToDevicePX4( &fHandle, 1 );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to connect to digitizer card: " );
        exit( -1 );
    }

    cout << "  *setting power up defaults..." << endl;

    tResult = SetPowerupDefaultsPX4( fHandle );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to enter default state: " );
        exit( -1 );
    }

    if( fChannelMode == 1 )
    {
        cout << "  *setting one active channel..." << endl;

        tResult = SetActiveChannelsPX4( fHandle, PX4CHANSEL_SINGLE_CH1 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to activate channel 1: " );
            exit( -1 );
        }
    }
    else if( fChannelMode == 2 )
    {
        cout << "  *setting two active channels..." << endl;

        tResult = SetActiveChannelsPX4( fHandle, PX4CHANSEL_DUAL_1_2 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to activate channels 1 and 2: " );
            exit( -1 );
        }
    }
    else
    {
        cout << "  *invalid channel mode setting <" << fChannelMode << ">" << endl;
        exit( -1 );
    }

    cout << "  *setting clock rate..." << endl;

    tResult = SetInternalAdcClockRatePX4( fHandle, fDigitizationRate );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to set sampling rate: " );
        exit( -1 );
    }

    cout << "  *allocating dma buffer of <" << fBufferCount << "> blocks with size <" << fRecordLength << ">..." << endl;

    for( size_t Index = 0; Index < fBuffer->fBufferCount; Index++ )
    {
        cout << "    *allocating block <" << Index << ">" << endl;

        tResult = AllocateDmaBufferPX4( fHandle, fRecordLength, &(fBuffer->fBufferArray[Index].fRecord.DataPtr()) );
        if( tResult != SIG_SUCCESS )
        {
            stringstream Converter;
            Converter << "    *failed to allocate block <" << Index << ">";
            DumpLibErrorPX4( tResult, Converter.str().c_str() );
            exit( -1 );
        }
    }

    return;
}

void MantisPX1500::Execute()
{
    int tResult;
    timeval tStartTime;
    timeval tStampTime;
    timeval tEndTime;
    timeval tDeadTime;

    //wait for the write condition to release me

    cout << "digitizer at initial block" << endl;

    fCondition.Wait();
    if( fStatus->IsRunning() == false )
    {
        return;
    }

    cout << "digitizer loose, starting acquisition..." << endl;

    //start acquisition
    if( StartAcquisition() == false )
    {
        return;
    }

    //start timing
    gettimeofday( &tStartTime, NULL );

    //go go go go
    while( true )
    {
        //check if we've been told to stop
        if( fStatus->IsRunning() == false )
        {
            //get the time and update the number of live microseconds
            gettimeofday( &tEndTime, NULL );
            fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);

            //halt the pci acquisition
            StopAcquisition();

            //GET OUT
            return;
        }

        fIterator->State()->SetAcquiring();

        gettimeofday( &tStampTime, NULL );

        fIterator->Record()->Index() = fRecordCount;
        fIterator->Record()->TimeStamp() = (1000000 * tEndTime.tv_sec + tEndTime.tv_usec);
        tResult = GetPciAcquisitionDataFastPX4( fHandle, ((unsigned int) (fRecordLength)), fIterator->Record()->DataPtr(), 0 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to acquire dma data over pci: " );
            tResult = EndBufferedPciAcquisitionPX4( fHandle );
            fStatus->SetError();
            return;
        }

        fIterator->State()->SetAcquired();

        if( fIterator->TryIncrement() == false )
        {
            //get the time and update the number of live microseconds
            gettimeofday( &tEndTime, NULL );
            fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);

            //halt the pci acquisition
            if( StopAcquisition() == false )
            {
                return;
            }

            cout << "digitizer stuck at block <" << fIterator->Index() << ">" << endl;

            //wait
            fCondition.Wait();

            //get the time and update the number of dead microseconds
            gettimeofday( &tDeadTime, NULL );
            fDeadMicroseconds += (1000000 * tDeadTime.tv_sec + tDeadTime.tv_usec) - (1000000 * tEndTime.tv_sec + tEndTime.tv_usec);

            if( fStatus->IsRunning() == false )
            {
                return;
            }

            cout << "digitizer loose at block <" << fIterator->Index() << ">" << endl;

            //start acquisition
            if( StartAcquisition() == false )
            {
                return;
            }
            fAcquisitionCount++;

            //start timing
            gettimeofday( &tStartTime, NULL );

            fIterator->Increment();
        }
    }

}

void MantisPX1500::Finalize()
{
    int Result;

    for( size_t Count = 0; Count < fBufferCount; Count++ )
    {
        Result = FreeDmaBufferPX4( fHandle, fIterator->Record()->DataPtr() );
        if( Result != SIG_SUCCESS )
        {
            DumpLibErrorPX4( Result, "failed to deallocate DMA buffer: " );
            exit( -1 );
        }
        fIterator->Increment();
    }

    Result = DisconnectFromDevicePX4( fHandle );
    if( Result != SIG_SUCCESS )
    {
        DumpLibErrorPX4( Result, "failed to disconnect from digitizer card: " );
        exit( -1 );
    }

    double LiveTime = fLiveMicroseconds / 1000000.;
    double DeadTime = fDeadMicroseconds / 1000000.;
    double MegabytesRead = fRecordCount * (((double) (fRecordLength)) / (1048576.));
    double ReadRate = MegabytesRead / LiveTime;

    cout << "\nreader statistics:\n";
    cout << "  * records taken: " << fRecordCount << "\n";
    cout << "  * aquisitions taken: " << fAcquisitionCount << "\n";
    cout << "  * live time: " << LiveTime << "(sec)\n";
    cout << "  * dead time: " << DeadTime << "(sec)\n";
    cout << "  * total data read: " << MegabytesRead << "(Mb)\n";
    cout << "  * average read rate: " << ReadRate << "(Mb/sec)\n";
    cout.flush();

    return;
}

bool MantisPX1500::StartAcquisition()
{
    int tResult = BeginBufferedPciAcquisitionPX4( fHandle, PX4_FREE_RUN );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to begin dma acquisition: " );
        fStatus->SetError();
        return false;
    }
    fAcquisitionCount++;
    return true;
}
bool MantisPX1500::StopAcquisition()
{
    int tResult = EndBufferedPciAcquisitionPX4( fHandle );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to end dma acquisition: " );
        fStatus->SetError();
        return false;
    }
    return true;
}
