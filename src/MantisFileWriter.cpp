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
}

MantisFileWriter* MantisFileWriter::writerFromEnv( safeEnvPtr& env )
{
    MantisFileWriter* NewFileWriter = new MantisFileWriter();
    NewFileWriter->fRecordLength = (env.get())->getRecordLength();

    NewFileWriter->fStatus->SetFileWriterCondition( &(NewFileWriter->fCondition) );

    return NewFileWriter;
}

void MantisFileWriter::Initialize()
{
    fStatus->SetFileWriterCondition( &fCondition );
    return;
}

void MantisFileWriter::Execute()
{
    //allocate some local variables
    bool tResult;
    timeval tStartTime;
    timeval tEndTime;

    //get an iterator and pull it up to right behind the read iterator
    while( fIterator->TryIncrement() == true )
        ;

    //wait for run to release me
    fCondition.Wait();
    if( fStatus->IsRunning() == false )
    {
        return;
    }

    //start timing
    gettimeofday( &tStartTime, NULL );

    //go go go
    while( true )
    {
        //check the run status
        if( !fStatus->IsRunning() )
        {
            //get the time and update the number of live microseconds
            gettimeofday( &tEndTime, NULL );
            fLiveMicroseconds += (1000000 * tEndTime.tv_sec + tEndTime.tv_usec) - (1000000 * tStartTime.tv_sec + tStartTime.tv_usec);
            return;
        }

        fIterator->State()->SetFlushing();

        tResult = fEgg->write_data( fIterator->Record() );
        if( tResult == false )
        {
            cout << "encountered error writing record <" << fRecordCount << ">" << endl;
            fStatus->SetError();
            return;
        }
        fRecordCount++;

        fIterator->State()->SetFree();

        if( fIterator->TryIncrement() == false )
        {
            if( fStatus->GetPX1500Condition()->IsWaiting() == true )
            {
                fStatus->GetPX1500Condition()->Release();
            }
            fIterator->Increment();
        }
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
