#include "MantisBuffer.hpp"

MantisBuffer::MantisBuffer() :
    fBufferArray( NULL ),
    fBufferCount( 0 ),
    fRecordLength( 0 )
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
    NewBuffer->fRecordLength = (env.get())->getRecordLength();

    NewBuffer->fBufferArray = new MantisBufferBlock[NewBuffer->fBufferCount];

    return NewBuffer;
}

MantisBufferIterator* MantisBuffer::CreateIterator() const
{
    return new MantisBufferIterator( fBufferArray, fBufferCount );
}
