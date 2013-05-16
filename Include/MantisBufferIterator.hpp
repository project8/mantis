#ifndef MANTISBUFFERITERATOR_HPP_
#define MANTISBUFFERITERATOR_HPP_

#include "MantisBufferBlock.hpp"

class MantisBufferIterator
{
    public:
        MantisBufferIterator( MantisBufferBlock* aBlockArray, const size_t& aBlockArraySize );
        ~MantisBufferIterator();

        const size_t& Index();

        void Increment();
        bool TryIncrement();

        void Decrement();
        bool TryDecrement();

        MantisBufferState* State();
        MantisBufferRecord* Record();

    protected:
        MantisBufferBlock* fBlockArray;
        const size_t fBlockArraySize;

        void IncrementIndex();
        void DecrementIndex();
        size_t fPreviousIndex;
        size_t fCurrentIndex;
        size_t fNextIndex;
};

inline MantisBufferState* MantisBufferIterator::State()
{
    return &fBlockArray[fCurrentIndex].fState;
}
inline MantisBufferRecord* MantisBufferIterator::Record()
{
    return &fBlockArray[fCurrentIndex].fRecord;
}

inline const size_t& MantisBufferIterator::Index()
{
    return fCurrentIndex;
}

inline void MantisBufferIterator::IncrementIndex()
{
    fPreviousIndex++;
    if( fPreviousIndex == fBlockArraySize )
    {
        fPreviousIndex = 0;
    }
    fCurrentIndex++;
    if( fCurrentIndex == fBlockArraySize )
    {
        fCurrentIndex = 0;
    }
    fNextIndex++;
    if( fNextIndex == fBlockArraySize )
    {
        fNextIndex = 0;
    }
    return;
}

inline void MantisBufferIterator::DecrementIndex()
{
    if( fPreviousIndex == 0 )
    {
        fPreviousIndex = fBlockArraySize;
    }
    fPreviousIndex--;
    if( fCurrentIndex == 0 )
    {
        fCurrentIndex = fBlockArraySize;
    }
    fCurrentIndex--;
    if( fNextIndex == 0 )
    {
        fNextIndex = fBlockArraySize;
    }
    fNextIndex--;
    return;
}

#endif
