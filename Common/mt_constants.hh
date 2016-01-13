/*
 * mt_constants.hh
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */

#ifndef MT_CONSTANTS_HH_
#define MT_CONSTANTS_HH_

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace mantis
{
    // Executable return constants

#define RETURN_SUCCESS 1
#define RETURN_ERROR -1
#define RETURN_CANCELED -2
#define RETURN_REVOKED -3

}

#endif /* MT_CONSTANTS_HH_ */
