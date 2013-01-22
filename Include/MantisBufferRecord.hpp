#ifndef MANTISBUFFERRECORD_HPP_
#define MANTISBUFFERRECORD_HPP_

#include <unistd.h>
#include "px1500.h"

class MantisBufferRecord
{
    public:
        typedef unsigned int AcquisitionIdType;
        typedef unsigned long int RecordIdType;
        typedef unsigned long int TimeType;
        typedef px4_sample_t DataType;

    public:
        MantisBufferRecord();
        ~MantisBufferRecord();

        AcquisitionIdType& AcquisitionId();
        RecordIdType& RecordId();
        TimeType& Time();
        DataType*& Data();

    private:
        AcquisitionIdType fAcquisitionId;
        RecordIdType fRecordId;
        TimeType fTime;
        DataType* fData;
};

inline MantisBufferRecord::AcquisitionIdType& MantisBufferRecord::AcquisitionId()
{
    return fAcquisitionId;
}
inline MantisBufferRecord::RecordIdType& MantisBufferRecord::RecordId()
{
    return fRecordId;
}
inline MantisBufferRecord::TimeType& MantisBufferRecord::Time()
{
    return fTime;
}
inline MantisBufferRecord::DataType*& MantisBufferRecord::Data()
{
    return fData;
}

#endif
