#define MANTIS_API_EXPORTS

#include "mt_digitizer.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "logger.hh"
#include "param.hh"
#include "mt_thread.hh"

#ifndef _WIN32
#include <unistd.h>
#endif

using scarab::param_value;

namespace mantis
{
    LOGGER( mtlog, "digitizer" );

    digitizer::digitizer() :
            f_params( NULL ),
            f_buffer( NULL ),
            f_buffer_condition( new condition() ),
            f_status( k_ok ),
            f_status_message()
    {
    }

    digitizer::~digitizer()
    {
        if( f_params != NULL ) delete [] f_params;
        delete f_buffer;
        delete f_buffer_condition;
    }

    bool digitizer::run_insitu_test()
    {
        DEBUG( mtlog, "calling allocate" );
        if( ! this->allocate() )
        {
            ERROR( mtlog, "failure during allocation" );
            return false;
        }

        DEBUG( mtlog, "calling initialize" );
        param_node t_global_config, t_dev_config;
        t_dev_config.add( "rate", param_value( 250.0 ) ); // MHz
        t_dev_config.add( "record-size", param_value( 8192 ) );
        t_global_config.add( "duration", param_value( 100.0 ) ); // ms
        if( !initialize( &t_global_config, &t_dev_config ) )
        {
            ERROR( mtlog, "failure during initialize" );
            return false;
        }

        thread t_digitizer_thread( this );

        DEBUG( mtlog, "calling execute");
        t_digitizer_thread.start();

        DEBUG( mtlog, "releasing" );
        f_buffer_condition->release();

        DEBUG( mtlog, "waiting" );
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif

        DEBUG( mtlog, "canceling" );
        t_digitizer_thread.cancel();

        DEBUG( mtlog, "calling finalize");
        param_node t_response;
        finalize( &t_response );
        INFO( mtlog, "In-situ digitizer test result:\n" << t_response );

        return true;
    }

}
