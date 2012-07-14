#include "MantisActor.hpp"

MantisActor::MantisActor() :
    fCondition(),
    fStatus( NULL ),
    fBuffer( NULL ),
    fIterator( NULL )
{
}

MantisActor::~MantisActor()
{
    if( fIterator != NULL )
    {
        delete fIterator;
        fIterator = NULL;
    }
}

