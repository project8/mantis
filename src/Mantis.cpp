#include "MantisEnv.hpp"
#include "MantisRun.hpp"
#include "MantisPX1500.hpp"
#include "MantisFileWriter.hpp"

#include "MantisThread.hpp"
#include <cstdlib>
#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

// The global environment variable.
safeEnvPtr runEnvironment;

int main( int argc, char** argv )
{
    runEnvironment = MantisEnv::parseArgs( argc, argv );

    std::cout << runEnvironment;

    MantisStatus* Status = new MantisStatus();
    MantisBuffer* Buffer = MantisBuffer::bufferFromEnv( runEnvironment );
    MantisRun* Run = MantisRun::runFromEnv( runEnvironment, Status );
    MantisPX1500* Reader = MantisPX1500::digFromEnv( runEnvironment, Status, Buffer );
    MantisEgg* OutputFile = MantisEgg::egg_from_env( runEnvironment );
    if( OutputFile )
    {
        OutputFile->write_header();
    }
    else
    {
        std::cout << "Couldn't create output file!" << std::endl;
        exit( 2 );
    }

    MantisFileWriter* Writer = MantisFileWriter::writerFromEnv( runEnvironment, OutputFile, Status, Buffer );

    Buffer->Initialize();
    Run->Initialize();

    Reader->Initialize();
    Writer->Initialize();

    MantisThread* RunThread = new MantisThread( Run );
    MantisThread* ReadThread = new MantisThread( Reader );
    MantisThread* WriteThread = new MantisThread( Writer );

    ReadThread->Start();
    while( Status->GetWriterCondition()->IsWaiting() == false )
        ;

    WriteThread->Start();
    while( Status->GetReaderCondition()->IsWaiting() == false )
        ;

    RunThread->Start();

    RunThread->Join();
    WriteThread->Join();
    ReadThread->Join();

    delete ReadThread;
    delete WriteThread;
    delete RunThread;

    Writer->Finalize();
    Reader->Finalize();
    Run->Finalize();
    Buffer->Finalize();

    delete Status;
    delete Buffer;
    delete Run;
    delete Reader;
    delete Writer;

    return 0;
}
