/*
 * MantisTime.hpp
 *
 *  Created on: May 13, 2013
 *      Author: nsoblath
 */

#ifndef MANTISTIME_HPP_
#define MANTISTIME_HPP_

#include <time.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000
#endif

inline int MantisTimeGetMonotonic(timespec* time)
{
    return clock_gettime( CLOCK_MONOTONIC, time );
}

inline long long int MantisTimeToNSec(timespec time)
{
    return (long long int)time.tv_sec * (long long int)NSEC_PER_SEC + (long long int)time.tv_nsec;
}

inline double MantisTimeToSec(timespec time)
{
    return (double)time.tv_sec + (double)time.tv_nsec / (double)NSEC_PER_SEC;
}

#endif /* MANTISTIME_HPP_ */
