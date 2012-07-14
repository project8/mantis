#include "MantisBufferState.hpp"

MantisBufferState::MantisBufferState() :
    fValue( eFree )
{
}
MantisBufferState::MantisBufferState( const MantisBufferState& aCopy ) :
    fValue( aCopy.fValue )
{
}
MantisBufferState::~MantisBufferState()
{
}

bool MantisBufferState::IsAcquiring() const
{
    return (fValue == eAcquiring);
}
void MantisBufferState::SetAcquiring()
{
	fValue = eAcquiring;
	return;
}

bool MantisBufferState::IsAcquired() const
{
	return (fValue == eAcquired);
}
void MantisBufferState::SetAcquired()
{
	fValue = eAcquired;
	return;
}

bool MantisBufferState::IsFlushing() const
{
	return (fValue == eFlushing);
}
void MantisBufferState::SetFlushing()
{
	fValue = eFlushing;
	return;
}

bool MantisBufferState::IsFree() const
{
	return (fValue == eFree);
}
void MantisBufferState::SetFree()
{
	fValue = eFree;
	return;
}
