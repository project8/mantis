#include "MantisFileWriter.hpp"

#include <time.h> // for clock_gettime(), time(), and other functions and data types
#include <cstring> // for memcpy()

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::stringstream;

MantisFileWriter::MantisFileWriter() :
    fMonarch( NULL ),
    fMonarchRecordInterleaved( NULL ),
    fPciRecordLength( 0 ),
    fRecordCount( 0 ),
    fLiveMicroseconds( 0 ),
    fFileName(""),
    fRunDuration( 0 ),
    fAcquisitionRate( 0. ),
    fRecordLength( 0 ),
    fChannelMode( 0 )
{
}
MantisFileWriter::~MantisFileWriter()
{
}

MantisFileWriter* MantisFileWriter::writerFromEnv( safeEnvPtr& tEnv )
{
    MantisFileWriter* NewFileWriter = new MantisFileWriter();

    NewFileWriter->fFileName = tEnv->getFileName();
    NewFileWriter->fRunDuration = tEnv->getRunDuration();
    NewFileWriter->fAcquisitionRate = tEnv->getAcquisitionRate();
    NewFileWriter->fRecordLength = tEnv->getRecordLength();
    NewFileWriter->fChannelMode = tEnv->getChannelMode();

    if( NewFileWriter->fChannelMode == 1 )
    {
        NewFileWriter->fPciRecordLength = 1 * tEnv->getRecordLength();
    }

    if( NewFileWriter->fChannelMode == 2 )
    {
        NewFileWriter->fPciRecordLength = 2 * tEnv->getRecordLength();
    }

    return NewFileWriter;
}

void MantisFileWriter::Initialize()
{
    timespec tTimeCal;
    time_t tRawTime;
    struct tm* tTimeInfo;
    const size_t tDateLength = 512;
    char tDateString[tDateLength];
    stringstream tDateAndTimeCal;

    clock_gettime( CLOCK_MONOTONIC, &tTimeCal );
    time( &tRawTime );
    tTimeInfo = localtime( &tRawTime );
    strftime( tDateString, tDateLength,  "%Y-%m-%d %H:%M:%S %z", tTimeInfo );

    tDateAndTimeCal << tDateString << " -- " << tTimeCal.tv_sec * 1000000000 + tTimeCal.tv_nsec;

    fMonarch = Monarch::OpenForWriting( fFileName );
    MonarchHeader* tHeader = fMonarch->GetHeader();
    tHeader->SetFilename( fFileName );
    tHeader->SetTimestamp( tDateAndTimeCal.str() );
    tHeader->SetDescription( "digitizer data" );
    tHeader->SetContentMode( sContentSignal );
    tHeader->SetSourceMode( sSourceMantis );
    if( fChannelMode == 1 )
    {
        tHeader->SetFormatMode( sFormatSingle );
    }
    if( fChannelMode == 2 )
    {
        tHeader->SetFormatMode( sFormatInterleavedDual );
    }
    tHeader->SetRate( fAcquisitionRate );
    tHeader->SetDuration( fRunDuration );
    tHeader->SetLength( fRecordLength );

    fMonarch->WriteHeader();
    fMonarch->SetInterface( sInterfaceInterleaved );
    fMonarchRecordInterleaved = fMonarch->GetRecordInterleaved();

    return;
}

void MantisFileWriter::Execute()
{
    MantisBufferIterator* fIterator = fBuffer->CreateIterator();

    //timeval tStartTime;
    //timeval tEndTime;
    timespec tStartTime;
    timespec tEndTime;

    while( fIterator->TryIncrement() == true )
        ;

    //cout << "file writer iterator in place" << endl;

    //start timing
    //gettimeofday( &tStartTime, NULL );
    clock_gettime( CLOCK_MONOTONIC, &tStartTime );

    //go go go
    while( true )
    {
        if( fIterator->TryIncrement() == false )
        {
            if( fCondition->IsWaiting() == true )
            {
                fCondition->Release();
            }
            fIterator->Increment();
        }

        //if the block we're on is open, the run is done
        if( fIterator->State()->IsFree() == true )
        {
            //cout << "file writer is finished" << endl;

            //get the time and update the number of live microseconds
            //gettimeofday( &tEndTime, NULL );
            //fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);
            clock_gettime( CLOCK_MONOTONIC, tEndTime );
            fLiveTime = Sum( fLiveTime, Diff( tEndTime, tStartTime ) );

            delete fIterator;
            return;
        }

        fIterator->State()->SetFlushing();

        //cout << "writing at <" << fIterator->Index() << ">" << endl;

        if( Flush( fIterator->Record() ) == false )
        {
            //GET OUT
            delete fIterator;
            return;
        }
        fRecordCount++;

        fIterator->State()->SetFree();
    }

    return;
}

void MantisFileWriter::Finalize()
{
    //double LiveTime = fLiveMicroseconds / 1000000.;
    double LiveTime = (double)fLiveTime.tv_sec + (double)fLiveTime.tv_nsec / 1000000000.;
    double MegabytesWritten = fRecordCount * (((double) (fRecordLength * fChannelMode)) / (1048576.));
    double WriteRate = MegabytesWritten / LiveTime;

    cout << "\nwriter statistics:\n";
    cout << "  * records written: " << fRecordCount << "\n";
    cout << "  * data written: " << MegabytesWritten << "(Mb)\n";
    cout << "  * live time: " << LiveTime << "(sec)\n";
    cout << "  * average write rate: " << WriteRate << "(Mb/sec)\n";
    cout.flush();

    return;
}

bool MantisFileWriter::Flush( MantisBufferRecord* aBufferRecord )
{
    fMonarchRecordInterleaved->fAcquisitionId = aBufferRecord->AcquisitionId();
    fMonarchRecordInterleaved->fRecordId = aBufferRecord->RecordId();
    fMonarchRecordInterleaved->fTime = aBufferRecord->Time();

    memcpy( fMonarchRecordInterleaved->fData, aBufferRecord->Data(), fPciRecordLength );

    if( fMonarch->WriteRecord() == false )
    {
        return false;
    }

    return true;
}

timespec MantisFileWriter::Diff(timespec start, timespec end) const{
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

timespec MantisFileWriter::Sum(timespec start, timespec diff) const{
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


