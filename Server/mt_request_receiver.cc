#include "mt_request_receiver.hh"

#include "mt_buffer.hh"
#include "mt_condition.hh"
#include "mt_logger.hh"
#include "mt_run_context_dist.hh"
#include "mt_run_queue.hh"
#include "mt_server.hh"
#include "mt_version.hh"

#include <cstddef>

using std::string;


namespace mantis
{
    MTLOGGER( mtlog, "request_receiver" );

    request_receiver::request_receiver( const param_node* a_config, server* a_server, run_queue* a_run_queue, condition* a_condition, const string& a_exe_name ) :
            f_config( *a_config ),
            f_server( a_server ),
            f_run_queue( a_run_queue ),
            f_condition( a_condition ),
            f_exe_name( a_exe_name ),
            f_buffer_size( 512 ),
            f_record_size( 419304 ),
            f_data_chunk_size( 1024 ),
            f_data_type_size( 1 ),
            f_bit_depth( 8 ),
            f_voltage_min( -0.25 ),
            f_voltage_range( 0.5 )
    {
    }

    request_receiver::~request_receiver()
    {
    }

    void request_receiver::execute()
    {
        run_context_dist* t_run_context;

        while( true )
        {
            t_run_context = new run_context_dist();
            MTINFO( mtlog, "waiting for incoming connections" );
            // thread is blocked by the accept call in server::get_connection 
            // until an incoming connection is received
            t_run_context->set_connection( f_server->get_connection() );

            MTINFO( mtlog, "receiving request..." );

            try
            {
                // use blocking option for pull request
                if( ! t_run_context->pull_request( MSG_WAITALL ) )
                {
                    MTERROR( mtlog, "unable to pull run request; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "unable to pull run request" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }

                // check version of client
                // major and minor versions must match
                unsigned t_server_major_ver = Mantis_VERSION_MAJOR;
                unsigned t_server_minor_ver = Mantis_VERSION_MINOR;
                //MTDEBUG( mtlog, "server major ver: " << t_server_major_ver << "; minor ver: " << t_server_minor_ver );
                request* t_request = t_run_context->lock_request_in();
                t_run_context->unlock_inbound();
                version t_client_version( t_request->client_version() );
                unsigned t_client_major_ver = t_client_version.major_version();
                unsigned t_client_minor_ver = t_client_version.minor_version();
                //MTDEBUG( mtlog, "client major ver: " << t_client_major_ver << "; minor ver: " << t_client_minor_ver );

                if( t_server_major_ver != t_client_major_ver || t_server_minor_ver != t_client_minor_ver )
                {
                    MTERROR( mtlog, "client and server software versions do not match:\n" <<
                            "\tServer: " << TOSTRING(Mantis_VERSION) << '\n' <<
                            "\tClient: " << t_client_version.version_str());
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    std::stringstream t_error_msg;
                    t_error_msg << "client (" << t_client_version.version_str() << ") and server (" << TOSTRING(Mantis_VERSION) << ") software versions do not match";
                    t_status->set_error_message( t_error_msg.str() );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }

                MTINFO( mtlog, "sending server status <acknowledged>..." );

                status* t_status = t_run_context->lock_status_out();
                t_status->set_state( status_state_t_acknowledged );
                t_status->set_buffer_size( f_buffer_size );
                t_status->set_record_size( f_record_size );
                t_status->set_data_chunk_size( f_data_chunk_size );
                t_status->set_data_type_size( f_data_type_size );
                t_status->set_bit_depth( f_bit_depth );
                t_status->set_voltage_min( f_voltage_min );
                t_status->set_voltage_range( f_voltage_range );
                t_status->set_server_exe( f_exe_name );
                t_status->set_server_version( "Mantis_VERSION" );
                t_status->set_server_commit( "Mantis_GIT_COMMIT" );
                string t_config_as_string;
                param_output_json::write_string( f_config, t_config_as_string, param_output_json::k_compact );
                t_status->set_server_config( t_config_as_string );

                t_run_context->push_status_no_mutex();
                t_run_context->unlock_outbound();

                MTINFO( mtlog, "waiting for client readiness..." );

                if( ! t_run_context->pull_client_status( MSG_WAITALL ) )
                {
                    MTERROR( mtlog, "unable to pull client status; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "unable to pull client status" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }
                client_status_state_t t_client_state = t_run_context->lock_client_status_in()->state();
                t_run_context->unlock_inbound();
                if( ! t_client_state == client_status_state_t_ready )
                {
                    MTERROR( mtlog, "client did not get ready; sending server status <error>" );
                    status* t_status = t_run_context->lock_status_out();
                    t_status->set_state( status_state_t_error );
                    t_status->set_error_message( "client is not ready" );
                    t_run_context->push_status_no_mutex();
                    t_run_context->unlock_outbound();
                    delete t_run_context->get_connection();
                    delete t_run_context;
                    continue;
                }
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "connection closed; detected in <" << cc.what() << ">" );
                delete t_run_context->get_connection();
                delete t_run_context;
                continue;
            }

            MTINFO( mtlog, "queuing request..." );

            t_run_context->lock_status_out()->set_state( status_state_t_waiting );
            t_run_context->unlock_outbound();
            f_run_queue->to_back( t_run_context );


            // if the queue condition is waiting, release it
            if( f_condition->is_waiting() == true )
            {
                //MTINFO( mtlog, "releasing queue condition" );
                f_condition->release();
            }

            //MTINFO( mtlog, "finished processing request" );
        }

        return;
    }

    void request_receiver::cancel()
    {
        return;
    }

    size_t request_receiver::get_buffer_size() const
    {
        return f_buffer_size;
    }
    void request_receiver::set_buffer_size( size_t size )
    {
        f_buffer_size = size;
        return;
    }

    size_t request_receiver::get_record_size() const
    {
        return f_record_size;
    }
    void request_receiver::set_record_size( size_t size )
    {
        f_record_size = size;
        return;
    }

    size_t request_receiver::get_data_chunk_size() const
    {
        return f_data_chunk_size;
    }
    void request_receiver::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }

    size_t request_receiver::get_data_type_size() const
    {
        return f_data_type_size;
    }
    void request_receiver::set_data_type_size( size_t size )
    {
        f_data_type_size = size;
        return;
    }

    size_t request_receiver::get_bit_depth() const
    {
        return f_bit_depth;
    }

    void request_receiver::set_bit_depth( size_t bd )
    {
        f_bit_depth = bd;
        return;
    }

    double request_receiver::get_voltage_min() const
    {
        return f_voltage_min;
    }

    void request_receiver::set_voltage_min( double v_min )
    {
        f_voltage_min = v_min;
        return;
    }

    double request_receiver::get_voltage_range() const
    {
        return f_voltage_range;
    }

    void request_receiver::set_voltage_range( double v_range )
    {
        f_voltage_range = v_range;
        return;
    }

}
