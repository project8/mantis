#ifndef MANTISBUFFER_HPP_
#define MANTISBUFFER_HPP_

#include "MantisEnv.hpp"
#include "MantisBufferBlock.hpp"
#include "MantisBufferIterator.hpp"

class MantisBuffer
{
    public:
        static MantisBuffer* bufferFromEnv( safeEnvPtr& env );
        virtual ~MantisBuffer();

        MantisBufferIterator* CreateIterator() const;

    private:
        MantisBuffer();

        MantisBufferBlock* fBufferArray;
        size_t fBufferCount;
        size_t fRecordLength;
};

#endif
