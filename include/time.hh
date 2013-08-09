#ifndef TIME_HH_
#define TIME_HH_

#include "types.hh"

#include <sys/time.h>
#include <cstddef>
#include <string>

namespace mantis
{

    inline timestamp_t get_integral_time()
    {
        timeval t_timeval;
        gettimeofday( &t_timeval, NULL );
        return 1000000 * t_timeval.tv_sec + t_timeval.tv_usec;
    }

    inline std::string get_string_time()
    {
        time_t t_raw_time;
        struct tm* t_processed_time;
        char t_string_time[ 512 ];

        time( &t_raw_time );
        t_processed_time = localtime( &t_raw_time );
        strftime( t_string_time, 512, "%Y-%m-%d %H:%M:%S %z", t_processed_time ); // sDateTimeFormat is defined in MonarchTypes.hpp
        return std::string( t_string_time );
    }

}

#endif
