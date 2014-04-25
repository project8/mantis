/*
 * mantis_client.cc
 *
 *      Author: Dan Furse
 *
 *  Client (file-writing) component of the DAQ
 *
 *  Usage:
 *  $> mantis_client host=<some host name> port=<some port number> file=<some file name> description=<describe your run> mode=<one or two channel> rate=<sampling rate> duration=<sampling duration>
 *
 *  Arguments:
 *  - host        (string; required):  address of the Mantis server host
 *  - port        (integer; required): port number opened by the server
 *  - file        (string; required):  egg filename
 *  - description (string; optional):  describe the run
 *  - mode        (integer; required): '1' for single-channel; '2' for double-channel
 *  - rate        (float; required):   digitization rate in MHz
 *  - duration    (float; required):   length of the run in ms
 *
 */

#include "mt_logger.hh"
#include "mt_client_config.hh"
#include "mt_configurator.hh"
#include "mt_run_client.hh"

using namespace mantis;


MTLOGGER( mtlog, "mantis_client" );


int main( int argc, char** argv )
{
    try
    {
        client_config t_cc;
        configurator t_configurator( argc, argv, &t_cc );

        run_client the_client( t_configurator.config(), t_configurator.exe_name() );

        the_client.execute();

        return the_client.get_return();
    }
    catch( mantis::exception& e )
    {
        MTERROR( mtlog, "mantis::exception caught: " << e.what() );
        return RETURN_ERROR;
    }
    catch( std::exception& e )
    {
        MTERROR( mtlog, "std::exception caught: " << e.what() );
        return RETURN_ERROR;
    }

    return RETURN_ERROR;
}

