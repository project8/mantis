#include "MantisStatus.hpp"

#include <iostream>
using std::cout;
using std::endl;

MantisStatus::MantisStatus() :
    fValue( eIdle ),
    fMutex(),
    fFileWriterCondition( NULL ),
    fPX1500Condition( NULL ),
    fRunCondition( NULL )
{
}
MantisStatus::~MantisStatus()
{
}

void MantisStatus::SetIdle()
{
    fMutex.Lock();
    fValue = eIdle;
    if( fPX1500Condition->IsWaiting() ) fPX1500Condition->Release();
    if( fFileWriterCondition->IsWaiting() ) fFileWriterCondition->Release();
    if( fRunCondition->IsWaiting() ) fRunCondition->Release();
    cout << "status is idle" << endl;
    fMutex.Unlock();
    return;
}
bool MantisStatus::IsIdle()
{
    fMutex.Lock();
    bool Value = (fValue == eIdle);
    fMutex.Unlock();
    return Value;
}

void MantisStatus::SetRunning()
{
    fMutex.Lock();
    fValue = eRunning;
    if( fPX1500Condition->IsWaiting() ) fPX1500Condition->Release();
    if( fFileWriterCondition->IsWaiting() ) fFileWriterCondition->Release();
    if( fRunCondition->IsWaiting() ) fRunCondition->Release();
    cout << "status is running" << endl;
    fMutex.Unlock();
    return;
}
bool MantisStatus::IsRunning()
{
    fMutex.Lock();
    bool Value = (fValue == eRunning);
    fMutex.Unlock();
    return Value;
}

void MantisStatus::SetError()
{
    fMutex.Lock();
    fValue = eError;
    if( fPX1500Condition->IsWaiting() ) fPX1500Condition->Release();
    if( fFileWriterCondition->IsWaiting() ) fFileWriterCondition->Release();
    if( fRunCondition->IsWaiting() ) fRunCondition->Release();
    cout << "status is error" << endl;
    fMutex.Unlock();
    return;
}
bool MantisStatus::IsError()
{
    fMutex.Lock();
    bool Value = (fValue == eError);
    fMutex.Unlock();
    return Value;
}

void MantisStatus::SetComplete()
{
    fMutex.Lock();
    fValue = eComplete;
    if( fPX1500Condition->IsWaiting() ) fPX1500Condition->Release();
    if( fFileWriterCondition->IsWaiting() ) fFileWriterCondition->Release();
    if( fRunCondition->IsWaiting() ) fRunCondition->Release();
    cout << "status is complete" << endl;
    fMutex.Unlock();
    return;
}
bool MantisStatus::IsComplete()
{
    fMutex.Lock();
    bool Value = (fValue == eComplete);
    fMutex.Unlock();
    return Value;
}
