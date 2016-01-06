#include "mt_configurator.hh"
#include "logger.hh"
#include "mt_client_tcp.hh"
#include "mt_client_config.hh"
#include "mt_run_context_dist.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

LOGGER( mtlog, "test_mantis_client" );

int analyze_status( run_context_dist* t_run_context )
{
    status_state_t t_state = t_run_context->lock_status_in()->state();
    t_run_context->unlock_inbound();
    switch( t_state )
    {
        case status_state_t_created :
            INFO( mtlog, "created..." << '\r');
            return 0;

        case status_state_t_acknowledged :
            INFO( mtlog, "acknowledged..." << '\r' );
            return 0;

        case status_state_t_waiting :
            INFO( mtlog, "waiting..." << '\r' );
            return 0;

        case status_state_t_started :
            INFO( mtlog, "started..." << '\r' );
            return 0;

        case status_state_t_running :
            INFO( mtlog, "running..." << '\r' );
            return 0;

        case status_state_t_stopped :
            INFO( mtlog, "stopped..." << '\r' );
            return 1;

        case status_state_t_error :
            INFO( mtlog, "error..." << '\r' );
            return -1;

        case status_state_t_canceled :
            INFO( mtlog, "canceled..." << '\r' );
            return -1;

        case status_state_t_revoked :
            INFO( mtlog, "revoked..." << '\r' );
            return -1;
    }
}

int main( int argc, char** argv )
{
    try
    {
        client_config t_cc;
        configurator t_configurator( argc, argv, &t_cc );

        string t_host = t_configurator.get< string >( "host" );
        INFO( mtlog, "attempting to reach host at: " << t_host );
        int t_port = t_configurator.get< int >( "port" );
        INFO( mtlog, "host port: " << t_port );
        client_tcp* t_client = new client_tcp( t_host, t_port );
        run_context_dist* t_run_context = new run_context_dist();
        t_run_context->set_connection( t_client );

        request* t_request = t_run_context->lock_request_out();
        t_request->set_write_host( "" );
        t_request->set_write_port( -1 );
        t_request->set_file( "/value/ohgod.egg" );
        t_request->set_description( "junk" );
        t_request->set_date( get_absolute_time_string() );
        t_request->set_mode( request_mode_t_single );
        t_request->set_rate( 800.0 );
        t_request->set_duration( 2000.0 );
        t_request->set_file_write_mode( request_file_write_mode_t_local );

        INFO( mtlog, "sending request...\n" << t_request->DebugString() );

        t_run_context->push_request_no_mutex();

        t_run_context->unlock_outbound();

        t_run_context->pull_status();
        analyze_status( t_run_context );

    //    do
    //    {
    //        t_run_context->pull_status();
    //    }
    //    while( analyze_status( t_run_context ) == 0 );

        INFO( mtlog, "done" );

        delete t_run_context;
        delete t_client;

        return 0;
    }
    catch( exception& e )
    {
        ERROR( "exception caught: " << e.what() );
    }
    return -1;
}

