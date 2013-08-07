#ifndef TIME_HH_
#define TIME_HH_

#include "types.hh"

#include <sys/time.h>
#include <cstddef>

namespace mantis
{

    timestamp_t get_time()
    {
        timeval t_timeval;
        gettimeofday( &t_timeval, NULL );
        return 1000000 * t_timeval.tv_sec + t_timeval.tv_usec;
    }

}

#endif
