#ifndef MANTISFILEWRITER_HH_
#define MANTISFILEWRITER_HH_

#include "MantisActor.hpp"
#include "MantisEnv.hpp"

#include "Monarch.hpp"
#include "MonarchTypes.hpp"
#include "MonarchHeader.hpp"
#include "MonarchRecord.hpp"

#include <string>
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

        string fFileName;
        unsigned int fRunDuration;
        double fAcquisitionRate;
        unsigned int fRecordLength;
        unsigned int fChannelMode;

        bool FlushOneChannel( MantisBufferRecord* tBufferRecord );
        bool FlushTwoChannel( MantisBufferRecord* tBufferRecord );
        bool (MantisFileWriter::*fFlushFunction)( MantisBufferRecord* tBufferRecord );

        Monarch* fMonarch;
        MonarchRecord* fMonarchRecordOne;
        MonarchRecord* fMonarchRecordTwo;
        unsigned long fRecordCount;
        unsigned long long fLiveMicroseconds;
};

#endif
