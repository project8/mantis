#ifndef MANTISBUFFERRECORD_HPP_
#define MANTISBUFFERRECORD_HPP_

#include <unistd.h>
#include "px1500.h"

class MantisBufferRecord
{
    public:
        typedef unsigned long int MantisAcquisitionIdType;
        typedef unsigned long int MantisRecordIdType;
        typedef long long int MantisTimeType;
        typedef px4_sample_t MantisDataType;

    public:
        MantisBufferRecord();
        ~MantisBufferRecord();

        MantisAcquisitionIdType& AcquisitionId();
        MantisRecordIdType& RecordId();
        MantisTimeType& Time();
        MantisDataType*& Data();

    private:
        MantisAcquisitionIdType fAcquisitionId;
        MantisRecordIdType fRecordId;
        MantisTimeType fTime;
        MantisDataType* fData;
};

inline MantisBufferRecord::MantisAcquisitionIdType& MantisBufferRecord::AcquisitionId()
{
    return fAcquisitionId;
}
inline MantisBufferRecord::MantisRecordIdType& MantisBufferRecord::RecordId()
{
    return fRecordId;
}
inline MantisBufferRecord::MantisTimeType& MantisBufferRecord::Time()
{
    return fTime;
}
inline MantisBufferRecord::MantisDataType*& MantisBufferRecord::Data()
{
    return fData;
}

#endif
