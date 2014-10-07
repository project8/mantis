/*
 * test_mantis_digitizer.cc
 *
 *  Created on: Feb 10, 2014
 *      Author: nsoblath
 */

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_configurator.hh"
#include "mt_digitizer.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"

#include "request.pb.h"

#include <string>
using std::string;

using namespace mantis;

MTLOGGER( mtlog, "test_mantis_digitizer" );


bool method_test(digitizer* t_digitizer)
{
    buffer* a_buffer;
    condition* a_condition;
    
    request* a_request;
    a_request->set_rate( 250.0 ); // MHz
    a_request->set_duration( 100.0 ); // ms
    
    MTDEBUG( mtlog, "call to allocate" );
    t_digitizer->allocate(a_buffer, a_condition);
    MTDEBUG( mtlog, "call to initialize" );
    t_digitizer->initialize( a_request );
}

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
    string t_test_type;
    t_test_type = t_config->get< string >( "testtype", "" );
    typedef std::conditional<(t_test_type == "method"), digitizer, test_digitizer>::type DIGITIZERTYPE;

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

    DIGITIZERTYPE* t_digitizer = NULL;
    factory< DIGITIZERTYPE >* t_dig_factory = NULL;
/*    if (t_test_type == "method" )
    {
        digitizer* t_digitizer = NULL;
        factory< digitizer >* t_dig_factory = NULL;
    } else {
        test_digitizer* t_digitizer = NULL;
        factory< test_digitizer >* t_dig_factory = NULL;
    }*/
    try
    {
        if (t_test_type == "method" )
        {
            t_dig_factory = factory< digitizer >::get_instance();
        } else {
            t_dig_factory = factory< test_digitizer >::get_instance();
        }
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

    if (t_test_type == "method" )
    {
        MTDEBUG( mtlog, "testing by call standard methods" );
        if( ! method_test(t_digitizer) )
        {
            MTERROR( mtlog, "test was unsuccessful!" );
            return -1;
        }
    }
    else
    {
        MTDEBUG( mtlog, "testing with solid block of code" );
        if( ! t_digitizer->run_test() )
        {
            MTERROR( mtlog, "test was unsuccessful!" );
            return -1;
        }
    }

    MTINFO( mtlog, "congratulations, digitizer <" << t_dig_name << "> passed the test!" );
    return 0;
}
