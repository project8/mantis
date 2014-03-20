/*
 * mt_run_client.cc
 *
 *  Created on: Mar 7, 2014
 *      Author: nsoblath
 */

#include "mt_run_client.hh"

#include "mt_client_file_writing.hh"
#include "mt_client_worker.hh"
#include "mt_client.hh"
#include "mt_exception.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_signal_handler.hh"
#include "mt_thread.hh"
#include "mt_version.hh"
#include "thorax.hh"

#include <algorithm> // for min
#include <string>
#include <unistd.h>
using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "run_client" );

    run_client::run_client( const param_node* a_node, const string& a_exe_name ) :
            f_config( *a_node ),
            f_exe_name( a_exe_name ),
            f_canceled( false ),
            f_return( 0 )
    {
    }

    run_client::~run_client()
    {
    }

    void run_client::execute()
    {
        MTINFO( mtlog, "creating request objects..." );

        bool t_client_writes_file = true;
        if( f_config.get_value< string >( "file-writer" ) == std::string( "server" ) )
        {
            t_client_writes_file = false;
        }

        string t_request_host = f_config.get_value< string >( "host" );
        int t_request_port = f_config.get_value< int >( "port" );

        string t_write_host;
        int t_write_port = -1;
        if( t_client_writes_file )
        {
            t_write_host = f_config.get_value< string >( "client-host" );
            t_write_port = f_config.get_value< int >( "client-port", t_request_port + 1 );
        }

        double t_duration = f_config.get_value< double >( "duration" );

        run_context_dist t_run_context;

        request* t_request = t_run_context.lock_request_out();
        t_request->set_write_host( t_write_host );
        t_request->set_write_port( t_write_port );
        t_request->set_file( f_config.get_value< string >( "file" ) );
        t_request->set_description( f_config.get_value< string >( "description", "default client run" ) );
        t_request->set_date( get_absolute_time_string() );
        t_request->set_mode( (request_mode_t)f_config.get_value< int >( "mode" ) );
        t_request->set_rate( f_config.get_value< double >( "rate" ) );
        t_request->set_duration( t_duration );
        t_request->set_file_write_mode( request_file_write_mode_t_local );
        t_request->set_client_exe( f_exe_name );
        t_request->set_client_version( "Mantis_VERSION" );
        t_request->set_client_commit( "Mantis_GIT_COMMIT" );
        if( t_client_writes_file )
        {
            t_request->set_file_write_mode( request_file_write_mode_t_remote );
        }
        t_run_context.unlock_outbound();

        // start the client for sending the request
        MTINFO( mtlog, "connecting with the server...");

        client* t_request_client;
        try
        {
            t_request_client = new client( t_request_host, t_request_port );
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to start client: " << e.what() );
            f_return = RETURN_ERROR;
            return;
        }

        t_run_context.set_connection( t_request_client );


        MTINFO( mtlog, "starting communicator" );

        thread t_comm_thread( &t_run_context );

        signal_handler t_sig_hand;

        try
        {
            t_sig_hand.push_thread( &t_comm_thread );

            t_comm_thread.start();
            t_run_context.wait_until_active();
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "an error occurred while running the communication thread" );
            f_return = RETURN_ERROR;
            return;
        }


        MTINFO( mtlog, "sending request..." )

        if( ! t_run_context.push_request() )
        {
            t_run_context.cancel();
            t_comm_thread.cancel();
            delete t_request_client;
            MTERROR( mtlog, "error sending request" );
            f_return = RETURN_ERROR;
            return;
        }


        setup_loop t_setup_loop( &t_run_context );
        thread t_setup_loop_thread( &t_setup_loop );
        t_sig_hand.push_thread( & t_setup_loop_thread );
        t_setup_loop_thread.start();
        t_setup_loop_thread.join();
        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread();
        }
        if( t_setup_loop.get_return() == RETURN_ERROR )
        {
            MTERROR( mtlog, "exiting due to error during setup loop" );
            t_run_context.cancel();
            t_comm_thread.cancel();
            delete t_request_client;
            f_return = RETURN_ERROR;
            return;
        }

        // Client has now received Acknowledged status from the server
        // Status message should now contain any information the client might need from the server
        // Server is now waiting for a client status update


        // get the data type size
        status* t_status = t_run_context.lock_status_in();
        //unsigned t_data_type_size = t_status->data_type_size();
        // record receiver is given data_type_size in client_file_writing's constructor
        f_config.add( "data-type-size", new param_value( t_status->data_type_size() ) );
        f_config.add( "bit-depth", new param_value( t_status->bit_depth() ) );
        f_config.add( "voltage-min", new param_value( t_status->voltage_min() ) );
        f_config.add( "voltage-range", new param_value( t_status->voltage_range() ) );
        t_run_context.unlock_inbound();

        /****************************************************************/
        /*********************** file writing ***************************/
        /****************************************************************/
        client_file_writing* t_file_writing = NULL;
        if( t_client_writes_file )
        {
            MTINFO( mtlog, "creating file-writing objects..." );

            try
            {
                t_file_writing = new client_file_writing( &f_config, &t_run_context, t_write_port );
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "error setting up file writing: " << e.what() );
                t_run_context.cancel();
                t_comm_thread.cancel();
                delete t_request_client;
                f_return = RETURN_ERROR;
                return;
            }

        }
        /****************************************************************/
        /****************************************************************/
        /****************************************************************/


        MTINFO( mtlog, "transmitting status: ready" )

        t_run_context.lock_client_status_out()->set_state( client_status_state_t_ready );

        bool t_push_result = t_run_context.push_client_status_no_mutex();
        t_run_context.unlock_outbound();
        if( ! t_push_result )
        {
            MTERROR( mtlog, "error sending client status" );
            delete t_file_writing;
            t_run_context.cancel();
            t_comm_thread.cancel();
            delete t_request_client;
            f_return = RETURN_ERROR;
            return;
        }

        run_loop t_run_loop( &t_run_context, t_file_writing );
        thread t_run_loop_thread( &t_run_loop );
        t_sig_hand.push_thread( & t_run_loop_thread );
        t_run_loop_thread.start();
        t_run_loop_thread.join();
        if( ! t_sig_hand.got_exit_signal() )
        {
            t_sig_hand.pop_thread();
        }
        int t_run_success = t_run_loop.get_return();
        if( t_run_success == RETURN_ERROR )
        {
            MTERROR( mtlog, "exiting due to error during run loop" );
            t_run_context.cancel();
            t_comm_thread.cancel();
            delete t_request_client;
            f_return = RETURN_ERROR;
            return;
        }

        /****************************************************************/
        /*********************** file writing ***************************/
        /****************************************************************/
        if( t_client_writes_file )
        {
            if( t_run_success < 0 )
            {
                t_file_writing->cancel();
            }

            MTINFO( mtlog, "waiting for record reception to end..." );

            t_file_writing->wait_for_finish();

            MTINFO( mtlog, "shutting down record receiver" );

            delete t_file_writing;
            t_file_writing = NULL;
        }
        /****************************************************************/
        /****************************************************************/
        /****************************************************************/



        if( t_run_success > 0 || t_run_success == RETURN_CANCELED )
        {
            response* t_response;
            // wait for a completed response from the server
            bool t_can_get_response = true;
            while( t_can_get_response )
            {
                t_response = t_run_context.lock_response_in();
                if( t_response->state() == response_state_t_complete ) break;
                t_run_context.unlock_inbound();

                t_can_get_response = t_run_context.wait_for_response();
            }

            if( t_can_get_response )
            {
                MTINFO( mtlog, "printing response from server..." );

                MTINFO( mtlog, "digitizer summary:\n"
                        << "  record count: " << t_response->digitizer_records() << " [#]\n"
                        << "  acquisition count: " << t_response->digitizer_acquisitions() << " [#]\n"
                        << "  live time: " << t_response->digitizer_live_time() << " [sec]\n"
                        << "  dead time: " << t_response->digitizer_dead_time() << " [sec]\n"
                        << "  megabytes: " << t_response->digitizer_megabytes() << " [Mb]\n"
                        << "  rate: " << t_response->digitizer_rate() << " [Mb/sec]\n");


                MTINFO( mtlog, "writer summary:\n"
                        << "  record count: " << t_response->writer_records() << " [#]\n"
                        << "  acquisition count: " << t_response->writer_acquisitions() << " [#]\n"
                        << "  live time: " << t_response->writer_live_time() << " [sec]\n"
                        << "  megabytes: " << t_response->writer_megabytes() << "[Mb]\n"
                        << "  rate: " << t_response->writer_rate() << " [Mb/sec]\n");

            }

            t_run_context.unlock_inbound();
        }

        t_run_context.cancel();
        t_comm_thread.cancel();
        delete t_request_client;

        f_return = t_run_success;
        return;
    }

    void run_client::cancel()
    {
        f_canceled.store( true );
        return;
    }

    int run_client::get_return()
    {
        return f_return;
    }




    run_client::setup_loop::setup_loop( run_context_dist* a_run_context ) :
            f_run_context( a_run_context ),
            f_canceled( false ),
            f_return( 0 )
    {}
    run_client::setup_loop::~setup_loop()
    {}

    void run_client::setup_loop::execute()
    {
        while( ! f_canceled.load() )
        {
            status_state_t t_state = f_run_context->lock_status_in()->state();
            f_run_context->unlock_inbound();

            if( t_state == status_state_t_acknowledged )
            {
                MTINFO( mtlog, "run request acknowledged...\n" );
                f_return = RETURN_SUCCESS;
                break;
            }
            else if( t_state == status_state_t_error )
            {
                MTERROR( mtlog, "error reported; run was not acknowledged\n" );
                f_return = RETURN_ERROR;
                break;
            }
            else if( t_state == status_state_t_revoked )
            {
                MTINFO( mtlog, "request revoked; run did not take place\n" );
                f_return = RETURN_ERROR;
                break;
            }
            else if( t_state != status_state_t_created )
            {
                MTERROR( mtlog, "server reported unusual status: " << t_state );
                f_return = RETURN_ERROR;
                break;
            }

            if( f_run_context->wait_for_status() )
                continue;

            MTERROR( mtlog, "(setup loop) unable to communicate with server" );
            f_return = RETURN_ERROR;
            break;
        }
        return;
    }

    void run_client::setup_loop::cancel()
    {
        f_canceled.store( true );
        return;
    }

    int run_client::setup_loop::get_return()
    {
        return f_return;
    }


    run_client::run_loop::run_loop( run_context_dist* a_run_context, client_file_writing* a_file_writing ) :
            f_run_context( a_run_context ),
            f_file_writing( a_file_writing ),
            f_canceled( false ),
            f_return( 0 )
    {}
    run_client::run_loop::~run_loop()
    {}

    void run_client::run_loop::execute()
    {
        while( ! f_canceled.load() )
        {
            status_state_t t_state = f_run_context->lock_status_in()->state();
            f_run_context->unlock_inbound();

            if( t_state == status_state_t_waiting )
            {
                MTINFO( mtlog, "waiting for run...\n" );
                //continue;
            }
            else if( t_state == status_state_t_started )
            {
                MTINFO( mtlog, "run has started...\n" );
                //continue;
            }
            else if( t_state == status_state_t_running )
            {
                MTINFO( mtlog, "run is in progress...\n" );
                //continue;
            }
            else if( t_state == status_state_t_stopped )
            {
                MTINFO( mtlog, "run status: stopped; data acquisition has finished\n" );
                f_return = RETURN_SUCCESS;
                break;
            }
            else if( t_state == status_state_t_error )
            {
                MTINFO( mtlog, "error reported; run did not complete\n" );
                f_return = RETURN_ERROR;
                break;
            }
            else if( t_state == status_state_t_canceled )
            {
                MTINFO( mtlog, "cancellation reported; some data may have been written\n" );
                f_return = RETURN_CANCELED;
                break;
            }
            else if( t_state == status_state_t_revoked )
            {
                MTINFO( mtlog, "request revoked; run did not take place\n" );
                f_return = RETURN_REVOKED;
                break;
            }
            else if( f_file_writing != NULL && f_file_writing->is_done() )
            {
                MTINFO( mtlog, "file writing is done, but run status still does not indicate run is complete"
                        << "                exiting run now!" );
                f_return = RETURN_CANCELED;
                break;
            }

            if( f_run_context->wait_for_status() )
                continue;

            MTERROR( mtlog, "(run loop) unable to communicate with server" );
            f_return = RETURN_ERROR;
            break;
        }
    }

    void run_client::run_loop::cancel()
    {
        f_canceled.store( true );
        return;
    }
    int run_client::run_loop::get_return()
    {
        return f_return;
    }


} /* namespace mantis */
