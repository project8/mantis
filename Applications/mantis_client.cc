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

#define MANTIS_API_EXPORTS
#define DRIPLINE_API_EXPORTS
#define SCARAB_API_EXPORTS

#include "dripline_version.hh"

#include "mt_constants.hh"
#include "mt_client_config.hh"
#include "mt_configurator.hh"
#include "mt_run_client.hh"
#include "mt_version.hh"

#include "logger.hh"

using namespace mantis;


LOGGER( mtlog, "mantis_client" );

set_version( mantis, version );

int main( int argc, char** argv )
{
    dripline::version_setter s_vsetter_mantis_version( new mantis::version() );
    try
    {
        client_config t_cc;
        configurator t_configurator( argc, argv, &t_cc );

        // Run the client

        run_client the_client( t_configurator.config() );

        the_client.execute();

        return the_client.get_return();
    }
    catch( scarab::error& e )
    {
        ERROR( mtlog, "configuration error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( exception& e )
    {
        ERROR( mtlog, "mantis error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( std::exception& e )
    {
        ERROR( mtlog, "std::exception caught: " << e.what() );
        return RETURN_ERROR;
    }

    return RETURN_ERROR;
}
