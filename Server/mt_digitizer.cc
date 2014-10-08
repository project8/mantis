#include "mt_digitizer.hh"

namespace mantis
{

    digitizer::digitizer() :
            f_params()
    {
    }

    digitizer::~digitizer()
    {
    }

    const dig_calib_params& digitizer::params() const
    {
        return f_params;
    }

    dig_calib_params& digitizer::params()
    {
        return f_params;
    }

    bool digitizer::run_insitu_test()
    {

    }

}
