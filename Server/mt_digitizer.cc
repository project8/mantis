#include "mt_digitizer.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"

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
        buffer a_buffer( 1, t_record_size );
        condition a_condition;
        if( ! this->allocate( &a_buffer, &a_condition ) )
        {
            MTERROR( mtlog, "failure during allocation" );
            return false;
        }

        MTDEBUG( mtlog, "calling initialize" );
        request* a_request = new request();
        a_request->set_rate( 250.0 ); // MHz
        a_request->set_duration( 100.0 ); // ms
        if ( ! this->initialize( a_request ) )
        {
            MTERROR( mtlog, "failure during initialize" );
            return false;
        }

        MTDEBUG( mtlog, "calling execute");
        execute();

        MTDEBUG( mtlog, "calling finalize");
        response* a_response;
        finalize( a_response );

        return true;
    }

}
