#include "MantisFileWriter.hpp"

#include <sys/time.h>

#include <iostream>
using std::cout;
using std::endl;

MantisFileWriter::MantisFileWriter() :
    fEgg( NULL ),
    fRecordCount( 0 ),
    fLiveMicroseconds( 0 )
{
}
MantisFileWriter::~MantisFileWriter()
{
    if( fEgg != NULL )
    {
        delete fEgg;
        fEgg = NULL;
    }
}

MantisFileWriter* MantisFileWriter::writerFromEnv( safeEnvPtr& env )
{
    MantisFileWriter* NewFileWriter = new MantisFileWriter();

    NewFileWriter->fRecordLength = (env.get())->getRecordLength();
    NewFileWriter->fEgg = MantisEgg::egg_from_env( env );

    return NewFileWriter;
}

void MantisFileWriter::Initialize()
{
    fStatus->SetFileWriterCondition( &fCondition );

    fEgg->write_header();

    return;
}

void MantisFileWriter::Execute()
{
    MantisBufferIterator* fIterator = fBuffer->CreateIterator();

    bool tResult;
    timeval tStartTime;
    timeval tEndTime;

    while( fIterator->TryIncrement() == true )
        ;

    cout << "file writer iterator in place" << endl;

    //start timing
    gettimeofday( &tStartTime, NULL );

    //go go go
    while( true )
    {
        if( fIterator->TryIncrement() == false )
        {
            if( fStatus->GetPX1500Condition()->IsWaiting() == true )
            {
                fStatus->GetPX1500Condition()->Release();
            }
            fIterator->Increment();
        }

        //if the block we're on is open, check the run status
        if( (fIterator->State()->IsFree() == true) && (fStatus->IsRunning() == false) )
        {
            cout << "file writer is finished" << endl;

            //get the time and update the number of live microseconds
            gettimeofday( &tEndTime, NULL );
            fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);

            delete fIterator;
            return;
        }

        fIterator->State()->SetFlushing();

        cout << "writing at <" << fIterator->Index() << ">" << endl;
        tResult = fEgg->write_data( fIterator->Record() );
        if( tResult == false )
        {
            //GET OUT
            fStatus->SetError();
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
    double MegabytesWritten = fRecordCount * (((double) (fRecordLength)) / (1048576.));
    double WriteRate = MegabytesWritten / LiveTime;

    cout << "\nwriter statistics:\n";
    cout << "  * records written: " << fRecordCount << "\n";
    cout << "  * live time: " << LiveTime << "(sec)\n";
    cout << "  * total data written: " << MegabytesWritten << "(Mb)\n";
    cout << "  * average write rate: " << WriteRate << "(Mb/sec)\n";
    cout.flush();

    return;
}
