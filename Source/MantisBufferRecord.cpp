#include "MantisBufferRecord.hpp"

MantisBufferRecord::MantisBufferRecord() :
    fAcquisitionId( 0 ),
    fRecordId( 0 ),
    fTime( 0 ),
    fData( NULL )
{
}

MantisBufferRecord::~MantisBufferRecord()
{
}
