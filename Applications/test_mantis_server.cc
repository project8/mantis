#include "mt_configurator.hh"
#include "logger.hh"
#include "mt_server_tcp.hh"
#include "mt_server_config.hh"
#include "mt_run_context_dist.hh"
#include "mt_connection.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

LOGGER( mtlog, "test_mantis_server" );

int main( int argc, char** argv )
{
    try
    {
        server_config t_sc;
        configurator t_configurator( argc, argv, &t_sc );

        INFO( mtlog, " starting server..." );

        server_tcp* t_server = new server_tcp( t_configurator.get< int >( "port" ) );
        run_context_dist* t_run_context = new run_context_dist();

        while( true )
        {

            INFO( mtlog, " waiting for connection..." );

            t_run_context->set_connection( t_server->get_connection() );

            t_run_context->pull_request();

            INFO( mtlog, " received request:\n" << t_run_context->lock_request_in()->DebugString() );

            status* t_status = t_run_context->lock_status_out();
            t_status->set_state( status_state_t_acknowledged );
            t_run_context->push_status();

            t_status->set_state( status_state_t_started );
            t_run_context->push_status();

            t_status->set_state( status_state_t_stopped );
            t_run_context->push_status();
            t_run_context->unlock_outbound();

            INFO( mtlog, " done" );

        }

        delete t_run_context->get_connection();
        delete t_run_context;
        delete t_server;

        return 0;
    }
    catch( exception& e )
    {
        ERROR( mtlog, "exception caught: " << e.what() );
    }
    return -1;
}
