#include "MantisBufferRecord.hpp"

MantisBufferRecord::MantisBufferRecord() :
    fAcquisitionId( 0 ),
    fRecordId( 0 ),
    fTimeStamp( 0 ),
    fDataPtr( NULL )
{
}

MantisBufferRecord::~MantisBufferRecord()
{
}
