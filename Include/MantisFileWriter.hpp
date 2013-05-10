#ifndef MANTISFILEWRITER_HH_
#define MANTISFILEWRITER_HH_

#include "MantisActor.hpp"
#include "MantisEnv.hpp"

#include "Monarch.hpp"
#include "MonarchTypes.hpp"
#include "MonarchHeader.hpp"
#include "MonarchRecord.hpp"

#include <string>
#include <time.h>
using std::string;

class MantisFileWriter :
    public MantisActor
{
    public:
        static MantisFileWriter* writerFromEnv( safeEnvPtr& env );
        virtual ~MantisFileWriter();

        void Initialize();
        void Execute();
        void Finalize();

    private:
        MantisFileWriter();

        Monarch* fMonarch;
        MonarchRecord* fMonarchRecordInterleaved;
        unsigned int fPciRecordLength;
        unsigned long fRecordCount;
        unsigned long long fLiveMicroseconds;
        timespec fLiveTime;

        string fFileName;
        unsigned int fRunDuration;
        double fAcquisitionRate;
        unsigned int fRecordLength;
        unsigned int fChannelMode;

        bool Flush( MantisBufferRecord* aBufferRecord );

        timespec Diff(timespec start, timespec end) const;
        timespec Sum(timespec start, timespec diff) const;
};

#endif
