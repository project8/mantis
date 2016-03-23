/*
 * test_mantis_config.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "mt_client_config.hh"
#include "mt_configurator.hh"
#include "logger.hh"

#include <iostream>

using namespace mantis;

LOGGER( mtlog, "test_mantis_config" );

int main( int argc, char** argv )
{
    client_config cc;
    configurator configurator( argc, argv, &cc );

    LINFO( mtlog, "configuration:\n" << configurator.config() );

    return 0;
}

