/*
 * mt_constants.hh
 *
 *  Created on: Jan 23, 2015
 *      Author: nsoblath
 */


namespace mantis
{
    // API export macros for windows
#ifdef _WIN32
#  ifdef MANTIS_API_EXPORTS
#    define MANTIS_API __declspec(dllexport)
#  else
#    define MANTIS_API __declspec(dllimport)
#  endif
#else
#  define MANTIS_API
#endif



    // Executable return constants

#define RETURN_SUCCESS 1
#define RETURN_ERROR -1
#define RETURN_CANCELED -2
#define RETURN_REVOKED -3



    // AMQP message constants
    // Conforming to the dripline standards defined in [dripline]/python/dripline/core/constants.py

    // Operation constants
#define OP_MANTIS_RUN    0 // == OP_SENSOR_SET
#define OP_MANTIS_QUERY  1 // == OP_SENSOR_GET
#define OP_MANTIS_CONFIG 6 // == OP_SENSOR_CONFIG

    // Message type constants
#define T_MANTIS_REPLY   2 // == T_REPLY
#define T_MANTIS_REQUEST 3 // == T_REQUEST
#define T_MANTIS_ALERT   4 // == T_ALERT
#define T_MANTIS_INFO    5 // == T_INFO

}

