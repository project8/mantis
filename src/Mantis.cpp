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

    MantisEgg* OutputFile = MantisEgg::egg_from_env( runEnvironment );
    MantisRun* Run = MantisRun::runFromEnv( runEnvironment );
    MantisPX1500* Reader = MantisPX1500::digFromEnv( runEnvironment );
    MantisFileWriter* Writer = MantisFileWriter::writerFromEnv( runEnvironment );

    Reader->SetStatus( Status );
    Reader->SetBuffer( Buffer );

    Writer->SetStatus( Status );
    Writer->SetBuffer( Buffer );

    Run->SetStatus( Status );

    Run->Initialize();
    Reader->Initialize();
    Writer->Initialize();

    if( OutputFile )
    {
        OutputFile->write_header();
    }
    else
    {
        std::cout << "could not create output file!" << std::endl;
        exit( 2 );
    }

    MantisThread* RunThread = new MantisThread( Run );
    MantisThread* ReadThread = new MantisThread( Reader );
    MantisThread* WriteThread = new MantisThread( Writer );

    ReadThread->Start();
    WriteThread->Start();
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

    delete Status;
    delete Buffer;
    delete Run;
    delete Reader;
    delete Writer;

    return 0;
}
