#ifndef MANTISSTATUS_HH_
#define MANTISSTATUS_HH_

#include "MantisMutex.hpp"
#include "MantisCondition.hpp"

#include <string>
using std::string;

class MantisStatus
{
    public:
        MantisStatus();
        ~MantisStatus();

        void SetFileWriterCondition( MantisCondition* aCondition );
        MantisCondition* GetFileWriterCondition();

        void SetPX1500Condition( MantisCondition* aCondition );
        MantisCondition* GetPX1500Condition();

        void SetRunCondition( MantisCondition* aCondition );
        MantisCondition* GetRunCondition();

        void Message( string aMessage );

        void SetIdle();
        bool IsIdle();

        void SetRunning();
        bool IsRunning();

        void SetComplete();
        bool IsComplete();

        void SetError();
        bool IsError();

    private:
        enum
        {
            eIdle, eRunning, eComplete, eError
        } fValue;
        MantisMutex fMutex;

        MantisCondition* fFileWriterCondition;
        MantisCondition* fPX1500Condition;
        MantisCondition* fRunCondition;
};

inline void MantisStatus::SetFileWriterCondition( MantisCondition* aCondition )
{
    fFileWriterCondition = aCondition;
    return;
}
inline MantisCondition* MantisStatus::GetFileWriterCondition()
{
    return fFileWriterCondition;
}

inline void MantisStatus::SetPX1500Condition( MantisCondition* aCondition )
{
    fPX1500Condition = aCondition;
    return;
}
inline MantisCondition* MantisStatus::GetPX1500Condition()
{
    return fPX1500Condition;
}

inline void MantisStatus::SetRunCondition( MantisCondition* aCondition )
{
    fRunCondition = aCondition;
    return;
}
inline MantisCondition* MantisStatus::GetRunCondition()
{
    return fRunCondition;
}

#endif
