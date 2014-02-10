/*
 * test_mantis_digitizer.cc
 *
 *  Created on: Feb 10, 2014
 *      Author: nsoblath
 */

#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"

#include <string>
using std::string;

using namespace mantis;

MTLOGGER( mtlog, "test_mantis_digitizer" );


int main( int argc, char** argv )
{
    configurator* t_config = NULL;
    try
    {
        t_config = new configurator( argc, argv );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "unable to configure test_mantis_digitizer: " << e.what() );
        return -1;
    }

    string t_dig_name;
    try
    {
        t_dig_name = t_config->get< string >( "digitizer" );
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "please provide the digitizer you want to test with configuration value <digitizer>" );
        return -1;
    }

    MTINFO( mtlog, "testing digitizer <" << t_dig_name << ">" );

    factory< test_digitizer >* t_dig_factory = NULL;
    test_digitizer* t_digitizer = NULL;
    try
    {
        t_dig_factory = factory< test_digitizer >::get_instance();
        t_digitizer = t_dig_factory->create( t_dig_name );
        if( t_digitizer == NULL )
        {
            MTERROR( mtlog, "could not create test_digitizer <" << t_dig_name << ">; aborting" );
            return -1;
        }
    }
    catch( exception& e )
    {
        MTERROR( mtlog, "exception caught while creating test_digitizer: " << e.what() );
        return -1;
    }

    if( ! t_digitizer->run_test() )
    {
        MTERROR( mtlog, "test was unsuccessful!" );
        return -1;
    }

    MTINFO( mtlog, "congratulations, digitizer <" << t_dig_name << "> passed the test!" );
    return 0;
}

