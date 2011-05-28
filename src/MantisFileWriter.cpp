#include "MantisFileWriter.hpp"

#include <cstdio>

#include <iostream>
using std::cout;
using std::endl;

MantisFileWriter::MantisFileWriter() :
    fCondition(), fFile( NULL ), fRecordCount( 0 ), fStatus( NULL ), fBuffer( NULL ), fFileName("")
{
}
MantisFileWriter::~MantisFileWriter()
{
}

void MantisFileWriter::SetStatus( MantisStatus* aStatus )
{
    fStatus = aStatus;
    return;
}
void MantisFileWriter::SetBuffer( MantisBuffer* aBuffer )
{
    fBuffer = aBuffer;
    return;
}
void MantisFileWriter::SetFileName( const string& aName )
{
    fFileName = aName;
    return;
}

void MantisFileWriter::Initialize()
{
    fStatus->SetReaderCondition( &fCondition );
    
    fFile = fopen( fFileName.c_str(), "w" );
    return;
}

void MantisFileWriter::Execute()
{
    //allocate some local variables
    int WriteResult;
    
    //get an iterator and pull it up to right behind the read iterator
    MantisBufferIterator* Iterator = fBuffer->CreateIterator();
    while( Iterator->TryIncrement() == true );
    
    //wait for run to release me
    fCondition.Wait();
    if( fStatus->IsRunning() == false )
    {
        delete Iterator;
        return;
    }

    //go go go
    while( true )
    {
        //check the run status
        if( !fStatus->IsRunning() )
        {
            delete Iterator;
            return;
        }
        
        Iterator->SetReading();
        WriteResult = fwrite( Iterator->Data()->fDataPtr, sizeof( MantisData::DataType ), fBuffer->GetDataLength(), fFile );
        fRecordCount++;
        Iterator->SetRead();
        
        if( Iterator->TryIncrement() == false )
        {
            if( fStatus->GetWriterCondition()->IsWaiting() == true )
            {
                cout << "found a waiting reader" << endl;
                fStatus->GetWriterCondition()->Release();
            }
            Iterator->Increment();
        }
    }
    return;
}
 
void MantisFileWriter::Finalize()
{
    fclose( fFile );
    
    cout << "records written: " << fRecordCount << endl;
    
    return;
}       
