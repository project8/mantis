/*
 * mt_types.hh
 *
 *  Created on: Jan 14, 2016
 *      Author: nsoblath
 */

#ifndef SERVER_MT_TYPES_HH_
#define SERVER_MT_TYPES_HH_

#include "time.hh"

#include "M3Types.hh"

namespace mantis
{
    typedef scarab::time_nsec_type time_nsec_type;

    typedef monarch3::byte_type byte_type;

    typedef monarch3::AcquisitionIdType acquisition_id_type;
    typedef monarch3::RecordIdType record_id_type;
    typedef monarch3::TimeType time_type;
}


#endif /* SERVER_MT_TYPES_HH_ */
