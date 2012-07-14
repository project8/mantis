#ifndef MANTISSTATUS_HH_
#define MANTISSTATUS_HH_

#include "MantisMutex.hpp"
#include "MantisCondition.hpp"

class MantisStatus
{
    public:
        MantisStatus();
        ~MantisStatus();

        void SetIdle();
        bool IsIdle();

        void SetRunning();
        bool IsRunning();

        void SetComplete();
        bool IsComplete();

        void SetError();
        bool IsError();

        void SetFileWriterCondition( MantisCondition* aCondition );
        MantisCondition* GetFileWriterCondition();

        void SetPX1500Condition( MantisCondition* aCondition );
        MantisCondition* GetPX1500Condition();

        void SetRunCondition( MantisCondition* aCondition );
        MantisCondition* GetRunCondition();

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

#endif
