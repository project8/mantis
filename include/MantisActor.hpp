#ifndef MANTISACTOR_HPP_
#define MANTISACTOR_HPP_

#include "MantisCallable.hpp"

#include "MantisCondition.hpp"
#include "MantisStatus.hpp"
#include "MantisBuffer.hpp"
#include "MantisBufferIterator.hpp"

class MantisActor :
    public MantisCallable
{
    public:
        MantisActor();
        virtual ~MantisActor();

        void SetStatus( MantisStatus* aStatus );
        void SetBuffer( MantisBuffer* aBuffer );

    protected:
        MantisCondition fCondition;
        MantisStatus* fStatus;
        MantisBuffer* fBuffer;
};

inline void MantisActor::SetStatus( MantisStatus* aStatus )
{
    fStatus = aStatus;
    return;
}
inline void MantisActor::SetBuffer( MantisBuffer* aBuffer )
{
    fBuffer = aBuffer;
    return;
}

#endif
