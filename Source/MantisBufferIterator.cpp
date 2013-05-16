#include "MantisBufferIterator.hpp"

#include <iostream>
using std::cout;
using std::endl;

MantisBufferIterator::MantisBufferIterator( MantisBufferBlock* aBlockArray, const size_t& aBlockArraySize ) :
    fBlockArray( aBlockArray ), fBlockArraySize( aBlockArraySize ), fPreviousIndex( aBlockArraySize - 1 ),fCurrentIndex( 0 ), fNextIndex( 1 )
{
    while( fBlockArray[fCurrentIndex].fMutex.Trylock() == false)
    {
        IncrementIndex();
    }
}
MantisBufferIterator::~MantisBufferIterator()
{
    fBlockArray[fCurrentIndex].fMutex.Unlock();
}

bool MantisBufferIterator::TryIncrement()
{
    if( fBlockArray[fNextIndex].fMutex.Trylock() == true )
    {
        fBlockArray[fCurrentIndex].fMutex.Unlock();
        IncrementIndex();
        return true;
    }
    else
    {
        return false;
    }
}
void MantisBufferIterator::Increment()
{
    fBlockArray[fNextIndex].fMutex.Lock();
    fBlockArray[fCurrentIndex].fMutex.Unlock();
    IncrementIndex();
    return;
}

bool MantisBufferIterator::TryDecrement()
{
    if( fBlockArray[fPreviousIndex].fMutex.Trylock() == true )
    {
        fBlockArray[fCurrentIndex].fMutex.Unlock();
        DecrementIndex();
        return true;
    }
    else
    {
        return false;
    }
}
void MantisBufferIterator::Decrement()
{
    fBlockArray[fPreviousIndex].fMutex.Lock();
    fBlockArray[fCurrentIndex].fMutex.Unlock();
    DecrementIndex();
    return;
}
