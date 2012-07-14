#ifndef MANTISBUFFERBLOCK_HPP_
#define MANTISBUFFERBLOCK_HPP_

#include "MantisBufferRecord.hpp"
#include "MantisBufferState.hpp"
#include "MantisMutex.hpp"

class MantisBufferBlock
{
    public:
        friend class MantisBufferIterator;

    public:
        MantisBufferBlock();
        ~MantisBufferBlock();

    public:
        MantisMutex fMutex;
        MantisBufferState fState;
        MantisBufferRecord fRecord;
};

#endif
