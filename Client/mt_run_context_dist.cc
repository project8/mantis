#include "mt_run_context_dist.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"

#include <cstdio>
#include <cstdlib>
#include <signal.h>

#include <google/protobuf/text_format.h>

namespace mantis
{
    MTLOGGER( mtlog, "run_context_dist" );

    const run_context_dist::message_id_type run_context_dist::f_unknown_id = 0;
    const run_context_dist::message_id_type run_context_dist::f_request_id = 1;
    const run_context_dist::message_id_type run_context_dist::f_status_id = 2;
    const run_context_dist::message_id_type run_context_dist::f_client_status_id = 3;
    const run_context_dist::message_id_type run_context_dist::f_response_id = 4;


    run_context_dist::run_context_dist() :
            f_request_out(),
            f_request_in(),
            f_status_out(),
            f_status_in(),
            f_client_status_out(),
            f_client_status_in(),
            f_response_out(),
            f_response_in(),
            f_outbound_mutex(),
            f_inbound_mutex_read(),
            f_inbound_mutex_write(),
            f_is_active( false ),
            f_is_canceled( false ),
            f_is_active_condition(),
            f_is_active_mutex(),
            f_request_condition(),
            f_status_condition(),
            f_client_status_condition(),
            f_response_condition()
    {
        f_status_out.set_state( status_state_t_created );
    }
    run_context_dist::~run_context_dist()
    {
    }

    void run_context_dist::execute()
    {
        f_is_active_mutex.lock();
        f_is_active.store( true );
        f_is_canceled.store( false );
        f_is_active_condition.release();
        f_is_active_mutex.unlock();

        while( ! f_is_canceled.load() )
        {
            int t_pull_result = pull_next_message( MSG_WAITALL );
            if( t_pull_result < 0 )
            {
                MTERROR( mtlog, "error in pulling message; aborting" );
                cancel();
                kill( 0, SIGINT );
            }
            else if( t_pull_result == 0 )
            {
                MTINFO( mtlog, "client/server connection has closed" );
                // set f_is_active to false here, before releasing any conditions
                // so that anything waiting on those conditions, when they're release and (presumably) wait again,
                // will get a false from the wait_for_[message type] function
                f_is_active.store( false );
                f_request_condition.release();
                f_status_condition.release();
                f_client_status_condition.release();
                f_response_condition.release();
                break;
            }
        }

        f_is_active.store( false );
        return;
    }

    void run_context_dist::wait_until_active()
    {
        f_is_active_mutex.lock();
        if( f_is_active.load() )
        {
	    f_is_active_mutex.unlock();
            return;
        }
        f_is_active_mutex.unlock();
        f_is_active_condition.wait();
        return;
    }

    void run_context_dist::cancel()
    {
        f_is_canceled.store( true );
        return;
    }

    int run_context_dist::pull_next_message( int flags )
    {
        try
        {
            f_inbound_mutex_write.lock();
            message_id_type t_type = f_connection->recv_type< message_id_type >( flags | MSG_PEEK );
            f_inbound_mutex_write.unlock();
            switch( t_type )
            {
                case f_request_id:
                    if( pull_request( flags ) ) return 1;
                    else return -1;
                    break;
                case f_status_id:
                    if( pull_status( flags ) ) return 1;
                    else return -1;
                    break;
                case f_client_status_id:
                    if( pull_client_status( flags ) ) return 1;
                    else return -1;
                    break;
                case f_response_id:
                    if( pull_response( flags ) ) return 1;
                    else return -1;
                    break;
                default:
                    MTERROR( mtlog, "unknown message id: <" << t_type << ">" );
                    return -1;
                    break;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed; detected in <" << cc.what() << ">" );
            return 0;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "unable to check message type: " << e.what() );
            return -1;
        }
        // should not reach here
        return false;
    }


    bool run_context_dist::push_request( int flags )
    {
        bool t_return = false;
        f_outbound_mutex.lock();
        t_return = push_request_no_mutex( flags );
        f_outbound_mutex.unlock();
        return t_return;
    }
    bool run_context_dist::push_request_no_mutex( int flags )
    {
        size_t t_request_size = reset_buffer_out( f_request_out.ByteSize() );
        if( ! f_request_out.SerializeToArray( f_buffer_out, t_request_size ) )
        {
            MTERROR( mtlog, "unable to serialize request message" );
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_request_id << " for request" );
            f_connection->send_type( f_request_id, flags );
            //cout << "sending request" );
            f_connection->send( f_buffer_out, t_request_size, flags );
            //cout << "request sent" );
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (request); detected in <" << cc.what() << ">" );
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a write error occurred while pushing a request: " << e.what() );
            return false;
        }
        return true;
    }
    bool run_context_dist::pull_request( int flags )
    {
        bool t_return = false;
        f_inbound_mutex_read.lock();
        t_return = pull_request_no_mutex( flags );
        f_inbound_mutex_read.unlock();
        return t_return;
    }
    bool run_context_dist::pull_request_no_mutex( int flags )
    {
        f_inbound_mutex_write.lock();
        size_t t_request_size = f_buffer_in_size;
        try
        {
            message_id_type t_msg_type = f_unknown_id;
            if( ! verify_message_type( f_request_id, t_msg_type, flags ) )
            {
                MTERROR( mtlog, "message type <" << t_msg_type << "> is not request (" << f_request_id << ")" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_request_size = f_connection->recv_type< size_t >( flags );
            if( t_request_size == 0 )
            {
                MTERROR( mtlog, "request message size was 0" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_request_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_request_size, flags );
            if( recv_ret <= 0 )
            {
                MTERROR( mtlog, "(request) connection read length was: " << recv_ret );
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (request); detected in <" << cc.what() << ">" );
            f_inbound_mutex_write.unlock();
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a read error occurred while pulling a request: " << e.what() );
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_request_in.ParseFromArray( f_buffer_in, t_request_size );
        f_inbound_mutex_write.unlock();
        f_request_condition.release();
        return t_return;
    }
    request* run_context_dist::lock_request_out()
    {
        f_outbound_mutex.lock();
        return &f_request_out;
    }
    request* run_context_dist::lock_request_in()
    {
        f_inbound_mutex_read.lock();
        return &f_request_in;
    }


    bool run_context_dist::push_status( int flags )
    {
        bool t_return = false;
        f_outbound_mutex.lock();
        t_return = push_status_no_mutex( flags );
        f_outbound_mutex.unlock();
        return t_return;
    }
    bool run_context_dist::push_status_no_mutex( int flags )
    {
        size_t t_status_size = reset_buffer_out( f_status_out.ByteSize() );
        if( ! f_status_out.SerializeToArray( f_buffer_out, t_status_size ) )
        {
            MTERROR( mtlog, "unable to serialize status message" );
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_status_id << " for status" );
            f_connection->send_type( f_status_id, flags );
            //cout << "sending status" );
            f_connection->send( f_buffer_out, t_status_size, flags );
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (status); detected in <" << cc.what() << ">" );
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a write error occurred while pushing a status: " << e.what() );
            return false;
        }
        return true;
    }
    bool run_context_dist::pull_status( int flags )
    {
        bool t_return = false;
        f_inbound_mutex_read.lock();
        t_return = pull_status_no_mutex( flags );
        f_inbound_mutex_read.unlock();
        return t_return;
    }
    bool run_context_dist::pull_status_no_mutex( int flags )
    {
        f_inbound_mutex_write.lock();
        size_t t_status_size = f_buffer_in_size;
        try
        {
            message_id_type t_msg_type = f_unknown_id;
            if( ! verify_message_type( f_status_id, t_msg_type, flags ) )
            {
                MTERROR( mtlog, "message type <" << t_msg_type << "> is not status (" << f_status_id << ")" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_status_size = f_connection->recv_type< size_t >( flags );
            if( t_status_size == 0 )
            {
                MTERROR( mtlog, "status message size was 0" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_status_size, flags );
            if( recv_ret <= 0 )
            {
                MTERROR( mtlog, "(status) the connection read length was: " << recv_ret );
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (status); detected in <" << cc.what() << ">" );
            f_inbound_mutex_write.unlock();
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a read error occurred while pulling a status: " << e.what() );
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_status_in.ParseFromArray( f_buffer_in, t_status_size );
        f_inbound_mutex_write.unlock();
        f_status_condition.release();
        return t_return;
    }
    status* run_context_dist::lock_status_out()
    {
        f_outbound_mutex.lock();
        return &f_status_out;
    }
    status* run_context_dist::lock_status_in()
    {
        f_inbound_mutex_read.lock();
        return &f_status_in;
    }

    bool run_context_dist::push_client_status( int flags )
    {
        bool t_return = false;
        f_outbound_mutex.lock();
        t_return = push_client_status_no_mutex( flags );
        f_outbound_mutex.unlock();
        return t_return;
    }
    bool run_context_dist::push_client_status_no_mutex( int flags )
    {
        size_t t_client_status_size = reset_buffer_out( f_client_status_out.ByteSize() );
        if( ! f_client_status_out.SerializeToArray( f_buffer_out, t_client_status_size ) )
        {
            MTERROR( mtlog, "unable to serialize client_status message" );
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_client_status_id << " for client_status" );
            f_connection->send_type( f_client_status_id, flags );
            //cout << "sending client_status" );
            f_connection->send( f_buffer_out, t_client_status_size, flags );
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (client_status); detected in <" << cc.what() << ">" );
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a write error occurred while pushing a client_status: " << e.what() );
            return false;
        }
        return true;
    }
    bool run_context_dist::pull_client_status( int flags )
    {
        bool t_return = false;
        f_inbound_mutex_read.lock();
        t_return = pull_client_status_no_mutex( flags );
        f_inbound_mutex_read.unlock();
        return t_return;
    }
    bool run_context_dist::pull_client_status_no_mutex( int flags )
    {
        f_inbound_mutex_write.lock();
        size_t t_client_status_size = f_buffer_in_size;
        try
        {
            message_id_type t_msg_type = f_unknown_id;
            if( ! verify_message_type( f_client_status_id, t_msg_type, flags ) )
            {
                MTERROR( mtlog, "message type <" << t_msg_type << "> is not client_status (" << f_client_status_id << ")" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_client_status_size = f_connection->recv_type< size_t >( flags );
            if( t_client_status_size == 0 )
            {
                MTERROR( mtlog, "client_status message size was 0" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_client_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_client_status_size, flags );
            if( recv_ret <= 0 )
            {
                MTERROR( mtlog, "(client_status) the connection read length was: " << recv_ret );
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (client_status); detected in <" << cc.what() << ">" );
            f_inbound_mutex_write.unlock();
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a read error occurred while pulling a client_status: " << e.what() );
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_client_status_in.ParseFromArray( f_buffer_in, t_client_status_size );
        f_inbound_mutex_write.unlock();
        f_client_status_condition.release();
        return t_return;
    }
    client_status* run_context_dist::lock_client_status_out()
    {
        f_outbound_mutex.lock();
        return &f_client_status_out;
    }
    client_status* run_context_dist::lock_client_status_in()
    {
        f_inbound_mutex_read.lock();
        return &f_client_status_in;
    }

    bool run_context_dist::push_response( int flags )
    {
        bool t_return = false;
        f_outbound_mutex.lock();
        t_return = push_response_no_mutex( flags );
        f_outbound_mutex.unlock();
        return t_return;
    }
    bool run_context_dist::push_response_no_mutex( int flags )
    {
        size_t t_response_size = reset_buffer_out( f_response_out.ByteSize() );
        if( ! f_response_out.SerializeToArray( f_buffer_out, t_response_size ) )
        {
            MTERROR( mtlog, "unable to serialize response message" );
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_response_id << " for response" );
            f_connection->send_type( f_response_id, flags );
            //cout << "sending response" );
            f_connection->send( f_buffer_out, t_response_size, flags );
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (response); detected in <" << cc.what() << ">" );
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a write error occurred while pushing a response: " << e.what() );
            return false;
        }
        return true;
    }
    bool run_context_dist::pull_response( int flags )
    {
        bool t_return = false;
        f_inbound_mutex_read.lock();
        t_return = pull_response_no_mutex( flags );
        f_inbound_mutex_read.unlock();
        return t_return;
    }
    bool run_context_dist::pull_response_no_mutex( int flags )
    {
        f_inbound_mutex_write.lock();
        size_t t_response_size = f_buffer_in_size;
        try
        {
            message_id_type t_msg_type = f_unknown_id;
            if( ! verify_message_type( f_response_id, t_msg_type, flags ) )
            {
                MTERROR( mtlog, "message type <" << t_msg_type << "> is not response (" << f_response_id << ")" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_response_size = f_connection->recv_type< size_t >( flags );
            if( t_response_size == 0 )
            {
                MTERROR( mtlog, "response message size was 0" );
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_response_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_response_size, flags );
            if( recv_ret <= 0 )
            {
                MTERROR( mtlog, "(response) the connection read length was: " << recv_ret );
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (response); detected in <" << cc.what() << ">" );
            f_inbound_mutex_write.unlock();
            throw cc;
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "a read error occurred while pulling a response: " << e.what() );
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_response_in.ParseFromArray( f_buffer_in, t_response_size );
        f_inbound_mutex_write.unlock();
        f_response_condition.release();
        return t_return;
    }
    response* run_context_dist::lock_response_out()
    {
        f_outbound_mutex.lock();
        return &f_response_out;
    }
    response* run_context_dist::lock_response_in()
    {
        f_inbound_mutex_read.lock();
        return &f_response_in;
    }

    bool run_context_dist::verify_message_type( run_context_dist::message_id_type a_type_wanted, run_context_dist::message_id_type& a_type_found, int flags )
    {
        // private function; appropriate mutexes should already be set

        // peek at the message type, without marking that data as read
        a_type_found = f_connection->recv_type< message_id_type >( flags | MSG_PEEK );
        if( a_type_found != a_type_wanted )
        {
            return false;
        }
        // if all is well, mark the message-type data as read
        f_connection->recv_type< message_id_type >( flags );
        return true;
    }

    void run_context_dist::unlock_outbound()
    {
        f_outbound_mutex.unlock();
        return;
    }
    void run_context_dist::unlock_inbound()
    {
        f_inbound_mutex_read.unlock();
        return;
    }

    bool run_context_dist::is_active()
    {
        return f_is_active.load();
    }

    bool run_context_dist::wait_for_request()
    {
        if( ! f_is_active.load() )
            return false;
        f_request_condition.wait();
        return true;
    }
    bool run_context_dist::wait_for_status()
    {
        if( ! f_is_active.load() )
            return false;
        f_status_condition.wait();
        return true;
    }
    bool run_context_dist::wait_for_client_status()
    {
        if( ! f_is_active.load() )
            return false;
        f_client_status_condition.wait();
        return true;
    }
    bool run_context_dist::wait_for_response()
    {
        if( ! f_is_active.load() )
            return false;
        f_response_condition.wait();
        return true;
    }

}
