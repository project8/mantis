/*
 * mt_atomic.hh
 *
 *  Created on: Dec 5, 2013
 *      Author: nsoblath
 */

#ifndef MT_ATOMIC_HH_
#define MT_ATOMIC_HH_

#include "boost/atomic.hpp"

namespace mantis
{
    typedef boost::atomic< bool > atomic_bool;
}


#endif /* MT_ATOMIC_HH_ */
