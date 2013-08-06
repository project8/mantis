#ifndef MANTISPX1500SOURCE_HPP_
#define MANTISPX1500SOURCE_HPP_

#include "MantisActor.hpp"

#include "MantisEnv.hpp"
#include "MantisBufferRecord.hpp"
#include <cstddef> // px1500.h refers to NULL without including cstddef
#include "px1500.h"

class MantisPX1500 :
    public MantisActor
{
    public:
        static MantisPX1500* digFromEnv( safeEnvPtr& env );
        virtual ~MantisPX1500();

        void Initialize();
        void Execute();
        void Finalize();

    private:
        MantisPX1500();

        HPX4 fHandle;
        unsigned long fPciRecordSize;
        unsigned long fRunDurationLastRecord;
        MantisBufferRecord::MantisAcquisitionIdType fAcquisitionCount;
        MantisBufferRecord::MantisRecordIdType fRecordCount;
        MantisBufferRecord::MantisTimeType fLiveTime; // in nsec
        MantisBufferRecord::MantisTimeType fDeadTime; // in nsec

        double fAcquisitionRate;
        unsigned int fChannelMode;
        unsigned int fRecordSize;
        size_t fBufferCount;

        bool StartAcquisition( MantisBufferRecord::MantisDataType* anAddress = NULL );
        bool Acquire( MantisBufferRecord::MantisDataType* anAddress );
        bool StopAcquisition();
};

#endif
