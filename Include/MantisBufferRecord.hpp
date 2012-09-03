#ifndef MANTISBUFFERRECORD_HPP_
#define MANTISBUFFERRECORD_HPP_

#include <unistd.h>
#include "px1500.h"

class MantisBufferRecord
{
    public:
        typedef unsigned int AcquisitionIdType;
        typedef unsigned long int RecordIdType;
        typedef unsigned long int TimeStampType;
        typedef px4_sample_t DataType;

    public:
        MantisBufferRecord();
        ~MantisBufferRecord();

        AcquisitionIdType& AcquisitionId();
        RecordIdType& RecordId();
        TimeStampType& TimeStamp();
        DataType*& DataPtr();

    private:
        AcquisitionIdType fAcquisitionId;
        RecordIdType fRecordId;
        TimeStampType fTimeStamp;
        DataType* fDataPtr;
};

inline MantisBufferRecord::AcquisitionIdType& MantisBufferRecord::AcquisitionId()
{
    return fAcquisitionId;
}
inline MantisBufferRecord::RecordIdType& MantisBufferRecord::RecordId()
{
    return fRecordId;
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
