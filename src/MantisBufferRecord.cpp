#include "MantisBufferRecord.hpp"

MantisBufferRecord::MantisBufferRecord() :
    fRecordId( 0 ),
    fAcquisitionId( 0 ),
    fTimeStamp( 0 ),
    fDataPtr( NULL )
{
}

MantisBufferRecord::~MantisBufferRecord()
{
}
