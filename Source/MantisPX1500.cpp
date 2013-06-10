#include "MantisPX1500.hpp"

#include "MantisTime.hpp"

#include <cstdlib> // for exit()
#include <cmath> // for ceil()

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

MantisPX1500::MantisPX1500() :
    fHandle(),
    fPciRecordSize( 0 ),
    fRunDurationLastRecord( 0 ),
    fAcquisitionCount( 0 ),
    fRecordCount( 0 ),
    fLiveTime(),
    fDeadTime(),
    fAcquisitionRate( 0. ),
    fChannelMode( 1 ),
    fRecordSize( 0 ),
    fBufferCount( 0 )
{
}
MantisPX1500::~MantisPX1500()
{
}

MantisPX1500* MantisPX1500::digFromEnv( safeEnvPtr& env )
{
    MantisPX1500* NewPX1500 = new MantisPX1500();
    MantisEnv* tEnv = env.get();

    NewPX1500->fAcquisitionRate = tEnv->getAcquisitionRate();
    NewPX1500->fChannelMode = tEnv->getChannelMode();
    NewPX1500->fRecordSize = tEnv->getRecordSize();
    NewPX1500->fBufferCount = tEnv->getBufferCount();

    if( NewPX1500->fChannelMode == 1 )
    {
        NewPX1500->fPciRecordSize = 1 * tEnv->getRecordSize();
    }
    if( NewPX1500->fChannelMode == 2 )
    {
        NewPX1500->fPciRecordSize = 2 * tEnv->getRecordSize();
    }
    NewPX1500->fRunDurationLastRecord = (unsigned long) (ceil( (double)( tEnv->getAcquisitionRate() * tEnv->getRunDuration() * 1.e3 )/(double)( tEnv->getRecordSize() ) ));

    return NewPX1500;
}

void MantisPX1500::Initialize()
{
    int tResult;

    //cout << "  *connecting to digitizer card..." << endl;

    tResult = ConnectToDevicePX4( &fHandle, 1 );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to connect to digitizer card: " );
        exit( -1 );
    }

    //cout << "  *setting power up defaults..." << endl;

    tResult = SetPowerupDefaultsPX4( fHandle );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to enter default state: " );
        exit( -1 );
    }

    if( fChannelMode == 1 )
    {
        //cout << "  *setting one active channel..." << endl;

        tResult = SetActiveChannelsPX4( fHandle, PX4CHANSEL_SINGLE_CH1 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to activate channel 1: " );
            exit( -1 );
        }
    }
    else if( fChannelMode == 2 )
    {
        //cout << "  *setting two active channels..." << endl;

        tResult = SetActiveChannelsPX4( fHandle, PX4CHANSEL_DUAL_1_2 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to activate channels 1 and 2: " );
            exit( -1 );
        }
    }

    //cout << "  *setting clock rate..." << endl;

    tResult = SetInternalAdcClockRatePX4( fHandle, fAcquisitionRate );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to set clock rate: " );
        exit( -1 );
    }

    //cout << "  *allocating dma buffer of <" << fBufferCount << "> blocks with size <" << fPciRecordSize << ">..." << endl;

    MantisBufferIterator* tIterator = fBuffer->CreateIterator();
    for( size_t Index = 0; Index < fBufferCount; Index++ )
    {
        tResult = AllocateDmaBufferPX4( fHandle, fPciRecordSize, &tIterator->Record()->Data() );
        if( tResult != SIG_SUCCESS )
        {
            stringstream Converter;
            Converter << "    *failed to allocate block <" << Index << ">";
            DumpLibErrorPX4( tResult, Converter.str().c_str() );
            exit( -1 );
        }
        tIterator->Increment();
    }
    delete tIterator;

    return;
}

void MantisPX1500::Execute()
{
    MantisBufferIterator* tIterator = fBuffer->CreateIterator();

    timespec tStartTime;
    timespec tStampTime;
    timespec tEndTime;
    timespec tDeadTime;

    //cout << "px1500 is waiting" << endl;

    fCondition->Wait();

    //cout << "px1500 is loose at <" << tIterator->Index() << ">" << endl;

    //start acquisition
    if( StartAcquisition( tIterator->Record()->Data() ) == false )
    {
        return;
    }

    //start timing
    MantisTimeGetMonotonic( &tStartTime );

    //go go go go
    while( true )
    {
        //check if we've written enough
        if( fRecordCount == fRunDurationLastRecord )
        {
            //cout << "px1500 is finished" << endl;

            //mark the block as free
            tIterator->State()->SetFree();

            //get the time and update the number of live microseconds
            MantisTimeGetMonotonic( &tEndTime );
            fLiveTime += MantisTimeToNSec( tEndTime ) - MantisTimeToNSec( tStartTime );

            //halt the pci acquisition
            StopAcquisition();

            //GET OUT
            delete tIterator;
            //cout << "exiting because reached last record" << endl;
            return;
        }

        tIterator->State()->SetAcquiring();

        MantisTimeGetMonotonic( &tStampTime );

        tIterator->Record()->RecordId() = fRecordCount;
        tIterator->Record()->AcquisitionId() = fAcquisitionCount;
        tIterator->Record()->Time() = MantisTimeToNSec( tStampTime );

        if( Acquire( tIterator->Record()->Data() ) == false )
        {
            //mark the block as free
            tIterator->State()->SetFree();

            //get the time and update the number of live microseconds
            MantisTimeGetMonotonic( &tEndTime );
            fLiveTime += MantisTimeToNSec( tEndTime ) - MantisTimeToNSec( tStartTime );

            //halt the pci acquisition
            StopAcquisition();

            //GET OUT
            delete tIterator;
            cout << "exiting because acquisition failed" << endl;
            return;
        }

        tIterator->State()->SetAcquired();

        if( tIterator->TryIncrement() == false )
        {
            //cout << "px1500 is blocked at <" << tIterator->Index() << ">" << endl;

            //get the time and update the number of live microseconds
            MantisTimeGetMonotonic( &tEndTime );
            fLiveTime += MantisTimeToNSec( tEndTime ) - MantisTimeToNSec( tStartTime );

            //halt the pci acquisition
            if( StopAcquisition() == false )
            {
                //GET OUT
                delete tIterator;
                cout << "exiting because stop acquisition failed after increment failed" << endl;
                return;
            }

            //wait
            fCondition->Wait();

            //get the time and update the number of dead microseconds
            MantisTimeGetMonotonic( &tDeadTime );
            fDeadTime += MantisTimeToNSec( tDeadTime ) - MantisTimeToNSec( tEndTime );

            //start acquisition
            if( StartAcquisition() == false )
            {
                //GET OUT
                delete tIterator;
                cout << "exiting because start acquisition failed after increment failed" << endl;
                return;
            }

            //start timing
            MantisTimeGetMonotonic( &tStartTime );

            tIterator->Increment();

            //cout << "px1500 is loose at <" << tIterator->Index() << ">" << endl;
        }
    }

    return;
}

void MantisPX1500::Finalize()
{
    int tResult;

    //cout << "  *deallocating dma buffer of <" << fBufferCount << "> blocks..." << endl;

    MantisBufferIterator* tIterator = fBuffer->CreateIterator();
    for( size_t Index = 0; Index < fBufferCount; Index++ )
    {
        tResult = FreeDmaBufferPX4( fHandle, tIterator->Record()->Data() );
        if( tResult != SIG_SUCCESS )
        {
            stringstream Converter;
            Converter << "    *failed to allocate block <" << Index << ">";
            DumpLibErrorPX4( tResult, Converter.str().c_str() );
            exit( -1 );
        }
        tIterator->Increment();
    }
    delete tIterator;

    tResult = DisconnectFromDevicePX4( fHandle );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to disconnect from digitizer card: " );
        exit( -1 );
    }

    double LiveTime = fLiveTime / (double)NSEC_PER_SEC;
    double DeadTime = fDeadTime / (double)NSEC_PER_SEC;
    double MegabytesRead = fRecordCount * (((double) (fPciRecordSize)) / (1048576.));
    double ReadRate = MegabytesRead / LiveTime;

    cout << "\npx1500 statistics:\n";
    cout << "  * records taken: " << fRecordCount << "\n";
    cout << "  * acquisitions taken: " << fAcquisitionCount << "\n";
    cout << "  * live time: " << LiveTime << "(sec)\n";
    cout << "  * dead time: " << DeadTime << "(sec)\n";
    cout << "  * total data read: " << MegabytesRead << "(Mb)\n";
    cout << "  * average acquisition rate: " << ReadRate << "(Mb/sec)\n";
    cout.flush();

    return;
}

bool MantisPX1500::StartAcquisition( MantisBufferRecord::MantisDataType* anAddress )
{
    int tResult = BeginBufferedPciAcquisitionPX4( fHandle, PX4_FREE_RUN );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to begin dma acquisition: " );
        return false;
    }

    // Do one acquisition to fully prime the digitizer
    if( anAddress != NULL )
    {
        // cout << "performing pre-acquisition" << endl;
        tResult = GetPciAcquisitionDataFastPX4( fHandle, ((unsigned int) (fPciRecordSize)), anAddress, 0 );
        if( tResult != SIG_SUCCESS )
        {
            DumpLibErrorPX4( tResult, "failed to acquire dma data over pci: " );
            tResult = EndBufferedPciAcquisitionPX4( fHandle );
            return false;
        }
    }

    return true;
}
bool MantisPX1500::Acquire( MantisBufferRecord::MantisDataType* anAddress )
{
    int tResult = GetPciAcquisitionDataFastPX4( fHandle, ((unsigned int) (fPciRecordSize)), anAddress, 0 );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to acquire dma data over pci: " );
        tResult = EndBufferedPciAcquisitionPX4( fHandle );
        return false;
    }
    fRecordCount++;
    return true;
}
bool MantisPX1500::StopAcquisition()
{
    int tResult = EndBufferedPciAcquisitionPX4( fHandle );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to end dma acquisition: " );
        return false;
    }
    fAcquisitionCount++;
    return true;
}
