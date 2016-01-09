/*
 * test_mantis_digitizer.cc
 *
 *  Created on: Feb 10, 2014
 *      Author: nsoblath
 *
 *  Usage: test_mantis_digitizer digitizer=[name] testtype=[type]
 *
 *    digitizer: the name of the digitizer you want to test (e.g. px1500, u1084a)
 *    testtype: (optional; default = basic) either "basic" or "insitu"
 *
 *  Examples:
 *
 *    test_mantis_digitizer digitizer=px1500
 *        -- will run the basic test on the px1500 digitizer
 *
 *    test_mantis_digitizer digitizer=u1084a testtype=insitu
 *        -- will run the in-situ test on the u1084a digitizer
 */

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "logger.hh"

#include <string>
using std::string;

using namespace mantis;

LOGGER( mtlog, "test_mantis_digitizer" );

enum test_type
{
    k_basic,
    k_insitu
};


int main( int argc, char** argv )
{
    configurator* t_config = NULL;
    try
    {
        t_config = new configurator( argc, argv );
    }
    catch( exception& e )
    {
        ERROR( mtlog, "unable to configure test_mantis_digitizer: " << e.what() );
        return -1;
    }

    string t_dig_name;
    try
    {
        t_dig_name = t_config->get< string >( "digitizer" );
    }
    catch( exception& e )
    {
        ERROR( mtlog, "please provide the digitizer you want to test with configuration value <digitizer>" );
        return -1;
    }

    string t_str_test_type = t_config->get< string >( "testtype", "basic" );
    test_type t_test_type = k_basic;
    if( t_str_test_type == "basic" )
    {
        t_test_type = k_basic;
    }
    else if( t_str_test_type == "insitu" )
    {
        t_test_type = k_insitu;
    }
    else
    {
        ERROR( mtlog, "invalid test type: " << t_str_test_type );
        return -1;
    }

    INFO( mtlog, "testing digitizer <" << t_dig_name << ">" );

    device_manager t_dev_mgr;
    t_dev_mgr.set_device( t_dig_name );
    digitizer* t_digitizer = t_dev_mgr.device();

    bool t_status = false;
    if( t_test_type == k_basic )
    {
        t_status = t_digitizer->run_basic_test();
    }
    else
    {
        t_status = t_digitizer->run_insitu_test();
    }

    if ( ! t_status )
    {
        WARN( mtlog, "test failed" );
        return -1;
    }


    INFO( mtlog, "congratulations, digitizer <" << t_dig_name << "> passed the test!" );
    return 0;
}
