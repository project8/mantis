#include "MantisActor.hpp"

MantisActor::MantisActor() :
    fCondition(),
    fStatus( NULL ),
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

