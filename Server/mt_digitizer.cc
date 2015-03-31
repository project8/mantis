#define MANTIS_API_EXPORTS

#include "mt_digitizer.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"
#include "mt_param.hh"
#include "mt_thread.hh"

#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

namespace mantis
{
    MTLOGGER( mtlog, "digitizer" );

    digitizer::digitizer() :
            f_params(),
            f_buffer( NULL ),
            f_buffer_condition( new condition() )
    {
    }

    digitizer::~digitizer()
    {
        delete f_buffer;
        delete f_buffer_condition;
    }

    const dig_calib_params& digitizer::params() const
    {
        return f_params;
    }

    dig_calib_params& digitizer::params()
    {
        return f_params;
    }

    buffer* digitizer::get_buffer()
    {
        return f_buffer;
    }

    condition* digitizer::get_buffer_condition()
    {
        return f_buffer_condition;
    }


    bool digitizer::run_insitu_test()
    {
        MTDEBUG( mtlog, "calling allocate" );
        if( ! this->allocate() )
        {
            MTERROR( mtlog, "failure during allocation" );
            return false;
        }

        MTDEBUG( mtlog, "calling initialize" );
        param_node t_global_config, t_dev_config;
        t_dev_config.add( "rate", param_value( 250.0 ) ); // MHz
        t_dev_config.add( "record-size", param_value( 8192 ) );
        t_global_config.add( "duration", param_value( 100.0 ) ); // ms
        if( !initialize( &t_global_config, &t_dev_config ) )
        {
            MTERROR( mtlog, "failure during initialize" );
            return false;
        }

        thread t_digitizer_thread( this );

        MTDEBUG( mtlog, "calling execute");
        t_digitizer_thread.start();

        MTDEBUG( mtlog, "releasing" );
        f_buffer_condition->release();

        MTDEBUG( mtlog, "waiting" );
#ifndef _WIN32
        sleep(1);
#else
        Sleep(1000);
#endif

        MTDEBUG( mtlog, "canceling" );
        t_digitizer_thread.cancel();

        MTDEBUG( mtlog, "calling finalize");
        param_node t_response;
        finalize( &t_response );
        MTINFO( mtlog, "In-situ digitizer test result:\n" << t_response );

        return true;
    }

}
