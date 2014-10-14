#include "mt_digitizer.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"
#include "mt_thread.hh"

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
        request t_request;
        t_request.set_rate( 250.0 ); // MHz
        t_request.set_duration( 100.0 ); // ms
        if ( ! initialize( &t_request ) )
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
        sleep(1);

        MTDEBUG( mtlog, "canceling" );
        t_digitizer_thread.cancel();

        MTDEBUG( mtlog, "calling finalize");
        response* a_response;
        finalize( a_response );

        return true;
    }

}
