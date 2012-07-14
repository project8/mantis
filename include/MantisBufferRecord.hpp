#ifndef MANTISBUFFERRECORD_HPP_
#define MANTISBUFFERRECORD_HPP_

#include <unistd.h>
#include "px1500.h"

class MantisBufferRecord
{
    public:
        typedef unsigned long int IndexType;
        typedef unsigned long int TimeStampType;
        typedef px4_sample_t DataType;

    public:
        MantisBufferRecord();
        ~MantisBufferRecord();

        IndexType& Index();
        TimeStampType& TimeStamp();
        DataType*& DataPtr();

    private:
        IndexType fIndex;
        TimeStampType fTimeStamp;
        DataType* fDataPtr;
};

inline MantisBufferRecord::IndexType& MantisBufferRecord::Index()
{
    return fIndex;
}
inline MantisBufferRecord::TimeStampType& MantisBufferRecord::TimeStamp()
{
    return fTimeStamp;
}
inline MantisBufferRecord::DataType*& MantisBufferRecord::DataPtr()
{
    return fDataPtr;
}

#endif
