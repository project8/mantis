/*
 * test_mantis_parser.cc
 *
 *  Created on: May 12, 2015
 *      Author: nsoblath
 */

#include "mt_logger.cc"
#include "mt_parser.cc"


using namespace mantis;

MTLOGGER( mtlog, "test_mantis_parser" );

int main()
{
    parsable t_null( "null?", "" );

    parsable t_string( "string?", "i am a string" );

    parsable t_fp( "double?", "1.5" );
    parsable t_fpwexp( "double?", "1e10" );

    parsable t_uint( "unsigned integer?", "98324" );
    parsable t_int( "signed integer?", "-17834" );

    parsable t_true( "boolean true?", "true" );
    parsable t_false( "boolean false?", "false" );

    parsable t_ipaddr( "string?", "10.0.0.13" );


    return 0;
}


