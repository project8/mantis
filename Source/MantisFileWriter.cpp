#include "MantisFileWriter.hpp"

#include <sys/time.h> // for gettimeofday()
#include <cstring> // for memcpy()

#include <iostream>
using std::cout;
using std::endl;

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
    fMonarch = Monarch::OpenForWriting( fFileName, sInterleavedMode );
    MonarchHeader* tHeader = fMonarch->GetHeader();
    tHeader->SetFilename( fFileName );
    tHeader->SetDate( "now" );
    tHeader->SetDescription( "digitizer data" );
    tHeader->SetContentMode( sSignalContent );
    tHeader->SetSourceMode( sMantisSource );
    if( fChannelMode == 1 )
    {
        tHeader->SetFormatMode( sSingleFormat );
    }
    if( fChannelMode == 2 )
    {
        tHeader->SetFormatMode( sInterleavedDualFormat );
    }
    tHeader->SetRate( fAcquisitionRate );
    tHeader->SetLength( fRecordLength );

    fMonarch->WriteHeader();
    fMonarchRecordInterleaved = fMonarch->GetRecordDualInterleaved();

    return;
}

void MantisFileWriter::Execute()
{
    MantisBufferIterator* fIterator = fBuffer->CreateIterator();

    timeval tStartTime;
    timeval tEndTime;

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

bool MantisFileWriter::Flush( MantisBufferRecord* aBufferRecord )
{
    fMonarchRecordInterleaved->fAcquisitionId = aBufferRecord->AcquisitionId();
    fMonarchRecordInterleaved->fRecordId = aBufferRecord->RecordId();
    fMonarchRecordInterleaved->fTimeStamp = aBufferRecord->TimeStamp();

    memcpy( fMonarchRecordInterleaved->fData, aBufferRecord->DataPtr(), fPciRecordLength );

    if( fMonarch->WriteRecord() == false )
    {
        return false;
    }

    return true;
}
