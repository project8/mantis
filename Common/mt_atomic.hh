/*
 * mt_atomic.hh
 *
 *  Created on: Dec 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_ATOMIC_HH_
#define MT_ATOMIC_HH_

#include "mt_constants.hh"

#include "boost/atomic.hpp"

namespace mantis
{
    typedef boost::atomic< bool > atomic_bool;

#ifdef _WIN32
    MANTIS_EXPIMP_TEMPLATE template class MANTIS_API boost::atomic< bool >;
#endif

}


#endif /* MT_ATOMIC_HH_ */
