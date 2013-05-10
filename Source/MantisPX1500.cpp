#include "MantisPX1500.hpp"

#include <time.h> // for clock_gettime()
#include <cstdlib> // for exit()
#include <cmath> // for ceil()

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

MantisPX1500::MantisPX1500() :
    fHandle(),
    fPciRecordLength( 0 ),
    fRunDurationLastRecord( 0 ),
    fAcquisitionCount( 0 ),
    fRecordCount( 0 ),
    //fLiveMicroseconds( 0 ),
    //fDeadMicroseconds( 0 ),
    fLiveTime(),
    fDeadTime(),
    fAcquisitionRate( 0. ),
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
    MantisEnv* tEnv = env.get();

    NewPX1500->fAcquisitionRate = tEnv->getAcquisitionRate();
    NewPX1500->fChannelMode = tEnv->getChannelMode();
    NewPX1500->fRecordLength = tEnv->getRecordLength();
    NewPX1500->fBufferCount = tEnv->getBufferCount();

    if( NewPX1500->fChannelMode == 1 )
    {
        NewPX1500->fPciRecordLength = 1 * tEnv->getRecordLength();
    }
    if( NewPX1500->fChannelMode == 2 )
    {
        NewPX1500->fPciRecordLength = 2 * tEnv->getRecordLength();
    }
    NewPX1500->fRunDurationLastRecord = (unsigned long) (ceil( (double)( tEnv->getAcquisitionRate() * tEnv->getRunDuration() * 1.e3 )/(double)( tEnv->getRecordLength() ) ));

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

    //cout << "  *allocating dma buffer of <" << fBufferCount << "> blocks with size <" << fPciRecordLength << ">..." << endl;

    MantisBufferIterator* tIterator = fBuffer->CreateIterator();
    for( size_t Index = 0; Index < fBufferCount; Index++ )
    {
        tResult = AllocateDmaBufferPX4( fHandle, fPciRecordLength, &tIterator->Record()->Data() );
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
    //timeval tStartTime;
    //timeval tStampTime;
    //timeval tEndTime;
    //timeval tDeadTime;

    //cout << "px1500 is waiting" << endl;

    fCondition->Wait();

    //cout << "px1500 is loose at <" << tIterator->Index() << ">" << endl;

    //start acquisition
    if( StartAcquisition() == false )
    {
        return;
    }

    //start timing
    //gettimeofday( &tStartTime, NULL );
    clock_gettime( CLOCK_MONOTONIC, &tStartTime );

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
            //gettimeofday( &tEndTime, NULL );
            //fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);
            clock_gettime( CLOCK_MONOTONIC, &tEndTime );
            fLiveTime = Sum( fLiveTime, Diff( tStartTime, tEndTime ) );

            //halt the pci acquisition
            StopAcquisition();

            //GET OUT
            delete tIterator;
            return;
        }

        tIterator->State()->SetAcquiring();

        //gettimeofday( &tStampTime, NULL );
        clock_gettime( CLOCK_MONOTONIC, &tStampTime );

        tIterator->Record()->RecordId() = fRecordCount;
        tIterator->Record()->AcquisitionId() = fAcquisitionCount;
        //tIterator->Record()->Time() = (1000000 * tEndTime.tv_sec + tEndTime.tv_usec);
        tIterator->Record()->TimeStamp() = (1000000000 * tEndTime.tv_sec + tEndTime.tv_nsec);

        if( Acquire( tIterator->Record()->Data() ) == false )
        {
            //mark the block as free
            tIterator->State()->SetFree();

            //get the time and update the number of live microseconds
            //gettimeofday( &tEndTime, NULL );
            //fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);
            clock_gettime( CLOCK_MONOTONIC, &tEndTime );
            fLiveTime = Sum( fLiveTime, Diff( tStartTime, tEndTime ) );

            //halt the pci acquisition
            StopAcquisition();

            //GET OUT
            delete tIterator;
            return;
        }

        tIterator->State()->SetAcquired();

        if( tIterator->TryIncrement() == false )
        {
            //cout << "px1500 is blocked at <" << tIterator->Index() << ">" << endl;

            //get the time and update the number of live microseconds
            //gettimeofday( &tEndTime, NULL );
            //fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);
            clock_gettime( CLOCK_MONOTONIC, &tEndTime );
            fLiveTime = Sum( fLiveTime, Diff( tStartTime, tEndTime ) );

            //halt the pci acquisition
            if( StopAcquisition() == false )
            {
                //GET OUT
                delete tIterator;
                return;
            }

            //wait
            fCondition->Wait();

            //get the time and update the number of dead microseconds
            //gettimeofday( &tDeadTime, NULL );
            //fDeadMicroseconds += (1000000 * tDeadTime.tv_sec + tDeadTime.tv_usec) - (1000000 * tEndTime.tv_sec + tEndTime.tv_usec);
            clock_gettime( CLOCK_MONOTONIC, &tDeadTime );
            fDeadTime = Sum( fDeadTime, Diff( tEndTime, tDeadTime ) );

            //start acquisition
            if( StartAcquisition() == false )
            {
                //GET OUT
                delete tIterator;
                return;
            }

            //start timing
            //gettimeofday( &tStartTime, NULL );
            clock_gettime( CLOCK_MONOTONIC, &tStartTime );

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

    //double LiveTime = fLiveMicroseconds / 1000000.;
    //double DeadTime = fDeadMicroseconds / 1000000.;
    double LiveTime = (double)fLiveTime.tv_sec + (double)fLiveTime.tv_nsec / 1000000000.;
    double DeadTime = (double)fDeadTime.tv_sec + (double)fDeadTime.tv_nsec / 1000000000.;
    double MegabytesRead = fRecordCount * (((double) (fPciRecordLength)) / (1048576.));
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

bool MantisPX1500::StartAcquisition()
{
    int tResult = BeginBufferedPciAcquisitionPX4( fHandle, PX4_FREE_RUN );
    if( tResult != SIG_SUCCESS )
    {
        DumpLibErrorPX4( tResult, "failed to begin dma acquisition: " );
        return false;
    }
    return true;
}
bool MantisPX1500::Acquire( MantisBufferRecord::DataType* anAddress )
{
    int tResult = GetPciAcquisitionDataFastPX4( fHandle, ((unsigned int) (fPciRecordLength)), anAddress, 0 );
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

timespec MantisPX1500::Diff(timespec start, timespec end) const
{
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec < 0)){
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

timespec MantisPX1500::Sum(timespec start, timespec diff) const
{
    timespec temp = start;
    temp.tv_nsec += diff.tv_nsec;
    while (temp.tv_nsec > 1000000000)
    {
        temp.tv_sec++;
        temp.tv_nsec -= 1000000000;
    }
    temp.tv_sec += diff.tv_sec;
    return temp;
}

