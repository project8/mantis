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

    cout << "making status and buffer..." << endl;

    MantisStatus* Status = new MantisStatus();
    MantisBuffer* Buffer = MantisBuffer::bufferFromEnv( runEnvironment );

    cout << "making outfile..." << endl;

    MantisEgg* OutputFile = MantisEgg::egg_from_env( runEnvironment );

    cout << "making run..." << endl;

    MantisRun* Run = MantisRun::runFromEnv( runEnvironment );

    cout << "making digitizer..." << endl;

    MantisPX1500* Reader = MantisPX1500::digFromEnv( runEnvironment );

    cout << "making writer..." << endl;

    MantisFileWriter* Writer = MantisFileWriter::writerFromEnv( runEnvironment );

    cout << "setting up digitizer..." << endl;

    Reader->SetStatus( Status );
    Reader->SetBuffer( Buffer );

    cout << "setting up writer..." << endl;

    Writer->SetStatus( Status );
    Writer->SetBuffer( Buffer );
    Writer->SetOutputEgg( OutputFile );

    cout << "setting up run..." << endl;

    Run->SetStatus( Status );

    cout << "initializing run..." << endl;

    Run->Initialize();

    cout << "initializing digitizer..." << endl;

    Reader->Initialize();

    cout << "initializing writer..." << endl;

    Writer->Initialize();

    cout << "writing header..." << endl;

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

    cout << "setting threads loose..." << endl;

    ReadThread->Start();
    WriteThread->Start();
    RunThread->Start();

    ReadThread->Join();
    WriteThread->Join();
    RunThread->Join();

    delete ReadThread;
    delete WriteThread;
    delete RunThread;

    Reader->Finalize();
    Writer->Finalize();
    Run->Finalize();

    delete Status;
    delete Buffer;
    delete Run;
    delete Reader;
    delete Writer;

    return 0;
}
