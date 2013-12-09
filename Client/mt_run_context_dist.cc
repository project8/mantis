#include "mt_run_context_dist.hh"

#include "mt_exception.hh"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <signal.h>

using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

namespace mantis
{
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
                    f_is_canceled( false )
    {
        f_status_out.set_state( status_state_t_created );
    }
    run_context_dist::~run_context_dist()
    {
    }

    void run_context_dist::execute()
    {
        f_is_active.store( true );
        f_is_canceled.store( false );

        while( ! f_is_canceled.load() )
        {
            //usleep( 500 );

            if(! pull_next_message( MSG_WAITALL ) )
            {
                cerr << "unable to communicate with the server; aborting" << endl;
                kill( 0, SIGINT );
            }
        }

        f_is_active.store( false );
        return;
    }

    void run_context_dist::cancel()
    {
        f_is_canceled.store( true );
        return;
    }

    bool run_context_dist::pull_next_message( int flags )
    {
        try
        {
            f_inbound_mutex_write.lock();
            message_id_type t_type = f_connection->recv_type< message_id_type >( flags | MSG_PEEK );
            f_inbound_mutex_write.unlock();
            switch( t_type )
            {
                case f_request_id:
                    return pull_request( flags );
                    break;
                case f_status_id:
                    return pull_status( flags );
                    break;
                case f_client_status_id:
                    return pull_client_status( flags );
                    break;
                case f_response_id:
                    return pull_response( flags );
                    break;
                default:
                    cerr << "[run_context_dist] unknown message id: <" << t_type << ">" << endl;
                    return false;
                    break;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] unable to check message type: " << e.what() << endl;
            return false;
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
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_request_id << " for request" << endl;
            f_connection->send_type( f_request_id, flags );
            //cout << "sending request" << endl;
            f_connection->send( f_buffer_out, t_request_size, flags );
            //cout << "request sent" << endl;
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a write error occurred while pushing a request: " << e.what() << endl;
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
                cerr << "[run_context_dist] message type <" << t_msg_type << "> is not request (" << f_request_id << ")" << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_request_size = f_connection->recv_type< size_t >( flags );
            if( t_request_size == 0 )
            {
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_request_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_request_size, flags );
            if( recv_ret <= 0 )
            {
                cout << "[run_context_dist] (request) connection read length was: " << recv_ret << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a request: " << e.what() << endl;
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_request_in.ParseFromArray( f_buffer_in, t_request_size );
        f_inbound_mutex_write.unlock();
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
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_status_id << " for status" << endl;
            f_connection->send_type( f_status_id, flags );
            //cout << "sending status" << endl;
            f_connection->send( f_buffer_out, t_status_size, flags );
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a write error occurred while pushing a status: " << e.what() << endl;
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
                cerr << "[run_context_dist] message type <" << t_msg_type << "> is not status (" << f_status_id << ")" << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_status_size = f_connection->recv_type< size_t >( flags );
            if( t_status_size == 0 )
            {
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_status_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (status) the connection read length was: " << recv_ret << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a status: " << e.what() << endl;
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_status_in.ParseFromArray( f_buffer_in, t_status_size );
        f_inbound_mutex_write.unlock();
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
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_client_status_id << " for client_status" << endl;
            f_connection->send_type( f_client_status_id, flags );
            //cout << "sending client_status" << endl;
            f_connection->send( f_buffer_out, t_client_status_size, flags );
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a write error occurred while pushing a client_status: " << e.what() << endl;
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
                cerr << "[run_context_dist] message type <" << t_msg_type << "> is not client_status (" << f_client_status_id << ")" << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_client_status_size = f_connection->recv_type< size_t >( flags );
            if( t_client_status_size == 0 )
            {
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_client_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_client_status_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (client_status) the connection read length was: " << recv_ret << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a client_status: " << e.what() << endl;
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_client_status_in.ParseFromArray( f_buffer_in, t_client_status_size );
        f_inbound_mutex_write.unlock();
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
            return false;
        }
        try
        {
            //cout << "attempting to send type " << f_response_id << " for response" << endl;
            f_connection->send_type( f_response_id, flags );
            //cout << "sending response" << endl;
            f_connection->send( f_buffer_out, t_response_size, flags );
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a write error occurred while pushing a response: " << e.what() << endl;
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
                cerr << "[run_context_dist] message type <" << t_msg_type << "> is not response (" << f_response_id << ")" << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
            t_response_size = f_connection->recv_type< size_t >( flags );
            if( t_response_size == 0 )
            {
                f_inbound_mutex_write.unlock();
                return false;
            }
            reset_buffer_in( t_response_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_response_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (response) the connection read length was: " << recv_ret << endl;
                f_inbound_mutex_write.unlock();
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a response: " << e.what() << endl;
            f_inbound_mutex_write.unlock();
            return false;
        }
        bool t_return = f_response_in.ParseFromArray( f_buffer_in, t_response_size );
        f_inbound_mutex_write.unlock();
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


}
