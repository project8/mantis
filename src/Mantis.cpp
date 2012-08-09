#include "MantisEnv.hpp"
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

    //cout << "making status and buffer..." << endl;

    MantisCondition* tCondition = new MantisCondition();
    MantisBuffer* tBuffer = MantisBuffer::bufferFromEnv( runEnvironment );

    //cout << "making px1500..." << endl;

    MantisPX1500* tPX1500 = MantisPX1500::digFromEnv( runEnvironment );

    //cout << "making file writer..." << endl;

    MantisFileWriter* tFileWriter = MantisFileWriter::writerFromEnv( runEnvironment );

    //cout << "setting up px1500..." << endl;

    tPX1500->SetCondition( tCondition );
    tPX1500->SetBuffer( tBuffer );

    //cout << "setting up file writer..." << endl;

    tFileWriter->SetCondition( tCondition );
    tFileWriter->SetBuffer( tBuffer );

    //cout << "initializing px1500..." << endl;

    tPX1500->Initialize();

    //cout << "initializing file writer..." << endl;

    tFileWriter->Initialize();

    MantisThread* ReadThread = new MantisThread( tPX1500 );
    MantisThread* WriteThread = new MantisThread( tFileWriter );

    //cout << "setting threads loose..." << endl;

    ReadThread->Start();
    WriteThread->Start();

    ReadThread->Join();
    WriteThread->Join();

    delete ReadThread;
    delete WriteThread;

    tPX1500->Finalize();
    tFileWriter->Finalize();

    delete tCondition;
    delete tBuffer;
    delete tPX1500;
    delete tFileWriter;

    return 0;
}
