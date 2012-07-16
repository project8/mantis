#ifndef MANTISFILEWRITER_HH_
#define MANTISFILEWRITER_HH_

#include "MantisActor.hpp"

#include "MantisEnv.hpp"
#include "MantisCondition.hpp"

#include "MantisEgg.hpp"
#include "MantisStatus.hpp"
#include "MantisBuffer.hpp"

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

        MantisEgg* fEgg;
        unsigned long fRecordCount;
        unsigned long long fLiveMicroseconds;

        size_t fRecordLength;
};

#endif
