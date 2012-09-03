#ifndef MANTISBUFFERITERATOR_HPP_
#define MANTISBUFFERITERATOR_HPP_

#include "MantisBufferBlock.hpp"

class MantisBufferIterator
{
    public:
        MantisBufferIterator( MantisBufferBlock* aBlockArray, const size_t& aBlockArrayLength );
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
        const size_t fBlockArrayLength;

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
    if( fPreviousIndex == fBlockArrayLength )
    {
        fPreviousIndex = 0;
    }
    fCurrentIndex++;
    if( fCurrentIndex == fBlockArrayLength )
    {
        fCurrentIndex = 0;
    }
    fNextIndex++;
    if( fNextIndex == fBlockArrayLength )
    {
        fNextIndex = 0;
    }
    return;
}

inline void MantisBufferIterator::DecrementIndex()
{
    if( fPreviousIndex == 0 )
    {
        fPreviousIndex = fBlockArrayLength;
    }
    fPreviousIndex--;
    if( fCurrentIndex == 0 )
    {
        fCurrentIndex = fBlockArrayLength;
    }
    fCurrentIndex--;
    if( fNextIndex == 0 )
    {
        fNextIndex = fBlockArrayLength;
    }
    fNextIndex--;
    return;
}

#endif
