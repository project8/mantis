#include "MantisExceptions.hpp"

const char* argument_exception::what()
{
    return "unknown error occurred while parsing arguments.";
}

clock_rate_exception::clock_rate_exception( std::string bad_rate ) :
    bad_clock_rate( bad_rate )
{ /* no-op */
}
clock_rate_exception::~clock_rate_exception() throw ()
{ /* no-op */
}
const char* clock_rate_exception::what()
{
    return ("invalid choice of clock rate: " + (*this).bad_clock_rate).c_str();
}

run_length_exception::run_length_exception( std::string bad_length ) :
    bad_run_length( bad_length )
{ /* no-op */
}
run_length_exception::~run_length_exception() throw ()
{ /* no-op */
}
const char* run_length_exception::what()
{
    return ("invalid choice of run length: " + (*this).bad_run_length).c_str();
}

channel_mode_exception::channel_mode_exception( std::string bad_mode ) :
    bad_channel_mode( bad_mode )
{ /* no-op */
}
channel_mode_exception::~channel_mode_exception() throw ()
{ /* no-op */
}
const char* channel_mode_exception::what()
{
    return ("invalid choice of channel mode: " + (*this).bad_channel_mode).c_str();
}
