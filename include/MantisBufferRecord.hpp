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

        IndexType& RecordId();
        IndexType& AcquisitionId();
        TimeStampType& TimeStamp();
        DataType*& DataPtr();

    private:
        IndexType fRecordId;
        IndexType fAcquisitionId;
        TimeStampType fTimeStamp;
        DataType* fDataPtr;
};

inline MantisBufferRecord::IndexType& MantisBufferRecord::RecordId()
{
    return fRecordId;
}
inline MantisBufferRecord::IndexType& MantisBufferRecord::AcquisitionId()
{
    return fAcquisitionId;
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
