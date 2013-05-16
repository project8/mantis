#include "MantisBuffer.hpp"

MantisBuffer::MantisBuffer() :
    fBufferArray( NULL ),
    fBufferCount( 0 ),
    fRecordSize( 0 )
{
}
MantisBuffer::~MantisBuffer()
{
    if( fBufferArray != NULL )
    {
        delete[] fBufferArray;
        fBufferArray = NULL;
    }
}

MantisBuffer* MantisBuffer::bufferFromEnv( safeEnvPtr& env )
{
    MantisBuffer* NewBuffer = new MantisBuffer();

    NewBuffer->fBufferCount = (env.get())->getBufferCount();
    NewBuffer->fRecordSize = (env.get())->getRecordSize();

    NewBuffer->fBufferArray = new MantisBufferBlock[NewBuffer->fBufferCount];

    return NewBuffer;
}

MantisBufferIterator* MantisBuffer::CreateIterator() const
{
    return new MantisBufferIterator( fBufferArray, fBufferCount );
}
