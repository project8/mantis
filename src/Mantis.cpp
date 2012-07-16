#include "MantisEnv.hpp"
#include "MantisStatus.hpp"
#include "MantisBuffer.hpp"
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

    cout << "making px1500..." << endl;

    MantisPX1500* Reader = MantisPX1500::digFromEnv( runEnvironment );

    cout << "making file writer..." << endl;

    MantisFileWriter* Writer = MantisFileWriter::writerFromEnv( runEnvironment );

    cout << "setting up px1500..." << endl;

    Reader->SetStatus( Status );
    Reader->SetBuffer( Buffer );

    cout << "setting up file writer..." << endl;

    Writer->SetStatus( Status );
    Writer->SetBuffer( Buffer );

    cout << "initializing px1500..." << endl;

    Reader->Initialize();

    cout << "initializing file writer..." << endl;

    Writer->Initialize();

    MantisThread* ReadThread = new MantisThread( Reader );
    MantisThread* WriteThread = new MantisThread( Writer );

    cout << "setting threads loose..." << endl;

    ReadThread->Start();
    WriteThread->Start();

    ReadThread->Join();
    WriteThread->Join();

    delete ReadThread;
    delete WriteThread;

    Reader->Finalize();
    Writer->Finalize();

    delete Status;
    delete Buffer;
    delete Reader;
    delete Writer;

    return 0;
}
