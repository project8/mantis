/*
 * test_mantis_config.cc
 *
 *  Created on: Nov 5, 2013
 *      Author: nsoblath
 */

#include "mt_client_config.hh"
#include "mt_configurator.hh"

#include <iostream>

using namespace mantis;

int main( int argc, char** argv )
{
    client_config cc;
    configurator configurator( argc, argv, &cc );

    configurator.config().print();

    return 0;
}

