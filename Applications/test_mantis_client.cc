#include "mt_parser.hh"
#include "mt_logger.hh"
#include "mt_client.hh"
#include "mt_run_context_dist.hh"
#include "thorax.hh"
using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

MTLOGGER( mtlog, "test_mantis_client" );

int analyze_status( run_context_dist* t_run_context )
{
    status_state_t t_state = t_run_context->lock_status_in()->state();
    t_run_context->unlock_inbound();
    switch( t_state )
    {
        case status_state_t_created :
            MTINFO( mtlog, "created..." << '\r');
            return 0;

        case status_state_t_acknowledged :
            MTINFO( mtlog, "acknowledged..." << '\r' );
            return 0;

        case status_state_t_waiting :
            MTINFO( mtlog, "waiting..." << '\r' );
            return 0;

        case status_state_t_started :
            MTINFO( mtlog, "started..." << '\r' );
            return 0;

        case status_state_t_running :
            MTINFO( mtlog, "running..." << '\r' );
            return 0;

        case status_state_t_stopped :
            MTINFO( mtlog, "stopped..." << '\r' );
            return 1;

        case status_state_t_error :
            MTINFO( mtlog, "error..." << '\r' );
            return -1;

        case status_state_t_canceled :
            MTINFO( mtlog, "canceled..." << '\r' );
            return -1;

        case status_state_t_revoked :
            MTINFO( mtlog, "revoked..." << '\r' );
            return -1;
    }
}

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    client* t_client = new client( t_parser.value_at( "host" )->get(), t_parser.value_at( "port" )->get< int >() );
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

    MTINFO( mtlog, "sending request...\n" << t_request->DebugString() );

    t_run_context->push_request_no_mutex();

    t_run_context->unlock_outbound();

    t_run_context->pull_status();
    analyze_status( t_run_context );

//    do
//    {
//        t_run_context->pull_status();
//    }
//    while( analyze_status( t_run_context ) == 0 );

    MTINFO( mtlog, "done" );

    delete t_run_context;
    delete t_client;

    return 0;
}

