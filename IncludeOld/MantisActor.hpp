#ifndef MANTISACTOR_HPP_
#define MANTISACTOR_HPP_

#include "MantisCallable.hpp"

#include "MantisCondition.hpp"
#include "MantisBuffer.hpp"

class MantisActor :
    public MantisCallable
{
    public:
        MantisActor();
        virtual ~MantisActor();

        void SetCondition( MantisCondition* aCondition );
        void SetBuffer( MantisBuffer* aBuffer );

    protected:
        MantisCondition* fCondition;
        MantisBuffer* fBuffer;
};

inline void MantisActor::SetCondition( MantisCondition* aCondition )
{
    fCondition = aCondition;
    return;
}
inline void MantisActor::SetBuffer( MantisBuffer* aBuffer )
{
    fBuffer = aBuffer;
    return;
}

#endif
