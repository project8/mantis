/*
 * mantis_server.cc
 *
 *      Author: Dan Furse
 *
 *  Server component of the DAQ for the any of the available digitizers.
 *  Default configuration options are given in class server_config (see mt_server_config.cc)
 *
 *  The server can be configured in three ways, each of which can override previous values:
 *   1. Default configuration (see class server_config in mt_server_config.cc)
 *   2. Configuration file supplied by the user (json-formatted)
 *   3. Command line options
 *
 *  The server requires the following configuration values:
 *   - port (integer; must match the port used by the client)
 *   - buffer-size (integer; number of record blocks in the DMA buffer)
 *   - record-size (integer; number of samples in a block **** NOTE: this sets the number of samples in a block, even though it's called "record-size" ****)
 *
 *  Usage:
 *  $> mantis_server config=config_file.json [further configuration]
 *
 *  Arguments:
 *  - config (string; optional): json-formatted configuration file
 *  - further configuration: override or add new values
 *    format: name/type=value
 *    e.g.:   port/i=8235
 */

#include "mt_configurator.hh"
#include "mt_constants.hh"
#include "logger.hh"
#include "mt_run_server.hh"
#include "mt_server_config.hh"
#include "mt_version.hh"

#include "dripline_version.hh"

using namespace mantis;

using std::string;

LOGGER( mtlog, "mantis_server" );

set_version( mantis, version );

int main( int argc, char** argv )
{
    LINFO( mtlog, "Welcome to Mantis\n\n" <<
            "\t\t _______  _______  _       __________________ _______ \n" <<
            "\t\t(       )(  ___  )( \\    /|\\__   __/\\__   __/(  ____ \\\n" <<
            "\t\t| () () || (   ) ||  \\  ( |   ) (      ) (   | (    \\/\n" <<
            "\t\t| || || || (___) ||   \\ | |   | |      | |   | (_____ \n" <<
            "\t\t| |(_)| ||  ___  || (\\ \\) |   | |      | |   (_____  )\n" <<
            "\t\t| |   | || (   ) || | \\   |   | |      | |         ) |\n" <<
            "\t\t| )   ( || )   ( || )  \\  |   | |   ___) (___/\\____) |\n" <<
            "\t\t|/     \\||/     \\||/    \\_)   )_(   \\_______/\\_______)\n\n");

    try
    {
        server_config t_sc;
        configurator t_configurator( argc, argv, &t_sc );

        version t_version;

        // Run the server

        run_server the_server( t_configurator.config(), &t_version );

        the_server.execute();

        return the_server.get_return();
    }
    catch( scarab::error& e )
    {
        LERROR( mtlog, "configuration error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( exception& e )
    {
        LERROR( mtlog, "mantis error: " << e.what() );
        return RETURN_ERROR;
    }
    catch( std::exception& e )
    {
        LERROR( mtlog, "std::exception caught: " << e.what() );
        return RETURN_ERROR;
    }
    catch( ... )
    {
        LERROR( mtlog, "unknown excpetion caught" );
        return RETURN_ERROR;
    }

    return RETURN_ERROR;
}

