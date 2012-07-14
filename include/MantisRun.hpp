#ifndef MANTISRUN_HPP_
#define MANTISRUN_HPP_

#include "MantisActor.hpp"
#include "MantisEnv.hpp"
#include "MantisStatus.hpp"

class MantisRun :
    public MantisActor
{
    public:
        static MantisRun* runFromEnv( safeEnvPtr& env );
        virtual ~MantisRun();

        void Initialize();
        void Execute();
        void Finalize();

    private:
        MantisRun();
        MantisStatus* fStatus;
        unsigned int fRunDuration;
};

#endif
