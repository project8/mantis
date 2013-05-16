#include "MantisFileWriter.hpp"

#include "MantisTime.hpp"

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
    fLiveTime( 0 ),
    fFileName(""),
    fRunDuration( 0 ),
    fAcquisitionRate( 0. ),
    fRecordSize( 0 ),
    fChannelMode( 0 ),
    fStartTimeMonotonic( 0 )
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
    NewFileWriter->fRecordSize = tEnv->getRecordSize();
    NewFileWriter->fChannelMode = tEnv->getChannelMode();

    if( NewFileWriter->fChannelMode == 1 )
    {
        NewFileWriter->fPciRecordLength = 1 * tEnv->getRecordSize();
    }

    if( NewFileWriter->fChannelMode == 2 )
    {
        NewFileWriter->fPciRecordLength = 2 * tEnv->getRecordSize();
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

    MantisTimeGetMonotonic( &tTimeCal );
    fStartTimeMonotonic = MantisTimeToNSec( tTimeCal );

    time( &tRawTime );
    tTimeInfo = localtime( &tRawTime );
    strftime( tDateString, tDateLength,  sDateTimeFormat.c_str(), tTimeInfo ); // sDateTimeFormat is defined in MonarchTypes.hpp
    tDateAndTimeCal << tDateString;

    fMonarch = Monarch::OpenForWriting( fFileName );
    MonarchHeader* tHeader = fMonarch->GetHeader();
    tHeader->SetFilename( fFileName );
    tHeader->SetTimestamp( tDateAndTimeCal.str() );
    tHeader->SetDescription( "digitizer data" );
    tHeader->SetRunType( sRunTypeSignal );
    tHeader->SetRunSource( sSourceMantis );
    tHeader->SetAcquisitionMode( fChannelMode );
    if( fChannelMode == 1 )
    {
        tHeader->SetFormatMode( sFormatSingle );
    }
    if( fChannelMode == 2 )
    {
        tHeader->SetFormatMode( sFormatMultiInterleaved );
    }
    tHeader->SetAcquisitionRate( fAcquisitionRate );
    tHeader->SetRunDuration( fRunDuration );
    tHeader->SetRecordSize( fRecordSize );

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
    MantisTimeGetMonotonic( &tStartTime );

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
            MantisTimeGetMonotonic( &tEndTime );
            fLiveTime += MantisTimeToNSec( tEndTime ) - MantisTimeToNSec( tStartTime );

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
    double LiveTime = fLiveTime / (double)NSEC_PER_SEC;
    double MegabytesWritten = fRecordCount * (((double) (fRecordSize * fChannelMode)) / (1048576.));
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
    fMonarchRecordInterleaved->fTime = aBufferRecord->Time() - fStartTimeMonotonic;

    memcpy( fMonarchRecordInterleaved->fData, aBufferRecord->Data(), fPciRecordLength );

    if( fMonarch->WriteRecord() == false )
    {
        return false;
    }

    return true;
}
