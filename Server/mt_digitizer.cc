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
            f_params()
    {
    }

    digitizer::~digitizer()
    {
    }

    const dig_calib_params& digitizer::params() const
    {
        return f_params;
    }

    dig_calib_params& digitizer::params()
    {
        return f_params;
    }

    bool digitizer::run_insitu_test()
    {
        unsigned t_record_size = 8192;

        MTDEBUG( mtlog, "calling allocate" );
        buffer t_buffer( 1, t_record_size );
        condition t_condition;
        if( ! this->allocate( &t_buffer, &t_condition ) )
        {
            MTERROR( mtlog, "failure during allocation" );
            return false;
        }

        MTDEBUG( mtlog, "calling initialize" );
        param_node t_config;
        t_config.add( "rate", param_value() << 250.0 ); // MHz
        t_config.add( "duration", param_value() << 100.0 ); // ms
        if ( ! initialize( &t_config ) )
        {
            MTERROR( mtlog, "failure during initialize" );
            return false;
        }

        thread t_digitizer_thread( this );

        MTDEBUG( mtlog, "calling execute");
        t_digitizer_thread.start();

        MTDEBUG( mtlog, "releasing" );
        t_condition.release();

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
