/*
 * dripline_api.hh
 *
 *  Created on: Jan 1, 2016
 *      Author: nsoblath
 */

#ifndef MANTIS_API_HH_
#define MANTIS_API_HH_

#include "scarab_api.hh"

namespace dripline
{
    // API export macros for windows
#ifdef _WIN32
#  ifdef MANTIS_API_EXPORTS
#    define MANTIS_API __declspec(dllexport)
#    define MANTIS_EXPIMP_TEMPLATE
#  else
#    define MANTIS_API __declspec(dllimport)
#    define MANTIS_EXPIMP_TEMPLATE extern
#  endif
#else
#  define MANTIS_API
#endif
}

#endif /* MANTIS_API_HH_ */
