#include "MantisFileWriter.hpp"

#include <sys/time.h> // for gettimeofday()
#include <cstring>

#include <iostream>
using std::cout;
using std::endl;

MantisFileWriter::MantisFileWriter() :
    fFileName(""),
    fRunDuration( 0 ),
    fAcquisitionRate( 0. ),
    fRecordLength( 0 ),
    fChannelMode( 0 ),
    fFlushFunction( &MantisFileWriter::FlushOneChannel ),
    fMonarch( NULL ),
    fMonarchRecordOne( NULL ),
    fMonarchRecordTwo( NULL ),
    fRecordCount( 0 ),
    fLiveMicroseconds( 0 )
{
}
MantisFileWriter::~MantisFileWriter()
{
}

MantisFileWriter* MantisFileWriter::writerFromEnv( safeEnvPtr& env )
{
    MantisFileWriter* NewFileWriter = new MantisFileWriter();

    NewFileWriter->fFileName = (env.get())->getFileName();
    NewFileWriter->fRunDuration = (env.get())->getRunDuration();
    NewFileWriter->fAcquisitionRate = (env.get())->getAcquisitionRate();
    NewFileWriter->fRecordLength = (env.get())->getRecordLength();
    NewFileWriter->fChannelMode = (env.get())->getChannelMode();

    if( NewFileWriter->fChannelMode == 1 )
    {
        NewFileWriter->fFlushFunction = &MantisFileWriter::FlushOneChannel;
    }

    if( NewFileWriter->fChannelMode == 2 )
    {
        NewFileWriter->fFlushFunction = &MantisFileWriter::FlushTwoChannel;
    }

    return NewFileWriter;
}

void MantisFileWriter::Initialize()
{
    fMonarch = Monarch::OpenForWriting( fFileName );


    MonarchHeader* tHeader = fMonarch->GetHeader();
    tHeader->SetFilename( fFileName );
    tHeader->SetAcqTime( fRunDuration );
    tHeader->SetAcqRate( fAcquisitionRate );
    tHeader->SetRecordSize( fRecordLength );
    if( fChannelMode == 1 )
    {
        tHeader->SetAcqMode( sOneChannel );
    }
    if( fChannelMode == 2 )
    {
        tHeader->SetAcqMode( sTwoChannel );
    }

    fMonarch->WriteHeader();

    return;
}

void MantisFileWriter::Execute()
{
    MantisBufferIterator* fIterator = fBuffer->CreateIterator();

    bool tResult;
    timeval tStartTime;
    timeval tEndTime;
    fMonarchRecordOne = fMonarch->GetRecordOne();
    fMonarchRecordTwo = fMonarch->GetRecordTwo();

    while( fIterator->TryIncrement() == true )
        ;

    //cout << "file writer iterator in place" << endl;

    //start timing
    gettimeofday( &tStartTime, NULL );

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
            gettimeofday( &tEndTime, NULL );
            fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);

            delete fIterator;
            return;
        }

        fIterator->State()->SetFlushing();

        //cout << "writing at <" << fIterator->Index() << ">" << endl;

        tResult = (this->*fFlushFunction)( fIterator->Record() );
        if( tResult == false )
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
    double LiveTime = fLiveMicroseconds / 1000000.;
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

bool MantisFileWriter::FlushOneChannel( MantisBufferRecord* aBufferRecord )
{
    fMonarchRecordOne->fCId = 1;
    fMonarchRecordOne->fAId = aBufferRecord->AcquisitionId();
    fMonarchRecordOne->fRId = aBufferRecord->RecordId();
    fMonarchRecordOne->fTick = aBufferRecord->TimeStamp();

    memcpy( fMonarchRecordOne->fDataPtr, aBufferRecord->DataPtr(), fRecordLength );

    if( fMonarch->WriteRecord() == false )
    {
        return false;
    }

    return true;
}

bool MantisFileWriter::FlushTwoChannel( MantisBufferRecord* aBufferRecord )
{
    fMonarchRecordOne->fCId = 1;
    fMonarchRecordOne->fAId = aBufferRecord->AcquisitionId();
    fMonarchRecordOne->fRId = aBufferRecord->RecordId();
    fMonarchRecordOne->fTick = aBufferRecord->TimeStamp();

    fMonarchRecordTwo->fCId = 2;
    fMonarchRecordTwo->fAId = aBufferRecord->AcquisitionId();
    fMonarchRecordTwo->fRId = aBufferRecord->RecordId();
    fMonarchRecordTwo->fTick = aBufferRecord->TimeStamp();

    MantisBufferRecord::DataType* tMantisPtr = aBufferRecord->DataPtr();
    DataType* tRecordOnePtr = fMonarchRecordOne->fDataPtr;
    DataType* tRecordTwoPtr = fMonarchRecordTwo->fDataPtr;

    for( size_t tIndex = 0; tIndex < fRecordLength; tIndex++ )
    {
        *tRecordOnePtr = *tMantisPtr;
        tRecordOnePtr++;
        tMantisPtr++;

        *tRecordTwoPtr = *tMantisPtr;
        tRecordTwoPtr++;
        tMantisPtr++;
    }

    if( fMonarch->WriteRecord() == false )
    {
        return false;
    }

    return true;
}
