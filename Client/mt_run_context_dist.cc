#include "mt_run_context_dist.hh"

#include "mt_exception.hh"

#include <iostream>
#include <cstdio>
#include <cstdlib>

using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

namespace mantis
{
    const run_context_dist::message_id_type run_context_dist::f_request_id = 1;
    const run_context_dist::message_id_type run_context_dist::f_status_id = 2;
    const run_context_dist::message_id_type run_context_dist::f_client_status_id = 3;
    const run_context_dist::message_id_type run_context_dist::f_response_id = 4;


    run_context_dist::run_context_dist() :
                    f_request(),
                    f_status(),
                    f_client_status(),
                    f_response()
    {
    }
    run_context_dist::~run_context_dist()
    {
    }

    bool run_context_dist::pull_next_message( int flags )
    {
        try
        {
            message_id_type t_type = f_connection->recv_type< message_id_type >( flags | MSG_PEEK );
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
        size_t t_request_size = reset_buffer( f_request.ByteSize() );
        if( ! f_request.SerializeToArray( f_buffer, t_request_size ) )
            return false;
        try
        {
            f_connection->send_type( f_request_id, flags );
            f_connection->send( f_buffer, t_request_size, flags );
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
        size_t t_request_size = f_buffer_size;
        try
        {
            if( ! verify_message_type( f_request_id, flags ) )
            {
                cerr << "[run_context_dist] message type is not request" << endl;
                return false;
            }
            t_request_size = f_connection->recv_type< size_t >( flags );
            if( t_request_size == 0 ) return false;
            reset_buffer( t_request_size );
            ssize_t recv_ret = f_connection->recv( f_buffer, t_request_size, flags );
            if( recv_ret <= 0 )
            {
                cout << "[run_context_dist] (request) connection read length was: " << recv_ret << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a request: " << e.what() << endl;
            return false;
        }
        return f_request.ParseFromArray( f_buffer, t_request_size );
    }
    request* run_context_dist::get_request()
    {
        return &f_request;
    }

    bool run_context_dist::push_status( int flags )
    {
        size_t t_status_size = reset_buffer( f_status.ByteSize() );
        if( ! f_status.SerializeToArray( f_buffer, t_status_size ) )
            return false;
        try
        {
            f_connection->send_type( f_status_id, flags );
            f_connection->send( f_buffer, t_status_size, flags );
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
        size_t t_status_size = f_buffer_size;
        try
        {
            if( ! verify_message_type( f_status_id, flags ) )
            {
                cerr << "[run_context_dist] message type is not status" << endl;
                return false;
            }
            t_status_size = f_connection->recv_type< size_t >( flags );
            if( t_status_size == 0 ) return false;
            reset_buffer( t_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer, t_status_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (status) the connection read length was: " << recv_ret << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a status: " << e.what() << endl;
            return false;
        }
        return f_status.ParseFromArray( f_buffer, t_status_size );
    }
    status* run_context_dist::get_status()
    {
        return &f_status;
    }

    bool run_context_dist::push_client_status( int flags )
    {
        size_t t_client_status_size = reset_buffer( f_client_status.ByteSize() );
        if( ! f_client_status.SerializeToArray( f_buffer, t_client_status_size ) )
            return false;
        try
        {
            f_connection->send_type( f_client_status_id, flags );
            f_connection->send( f_buffer, t_client_status_size, flags );
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
        size_t t_client_status_size = f_buffer_size;
        try
        {
            if( ! verify_message_type( f_client_status_id, flags ) )
            {
                cerr << "[run_context_dist] message type is not client_status" << endl;
                return false;
            }
            t_client_status_size = f_connection->recv_type< size_t >( flags );
            if( t_client_status_size == 0 ) return false;
            reset_buffer( t_client_status_size );
            ssize_t recv_ret = f_connection->recv( f_buffer, t_client_status_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (client_status) the connection read length was: " << recv_ret << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a client_status: " << e.what() << endl;
            return false;
        }
        return f_client_status.ParseFromArray( f_buffer, t_client_status_size );
    }
    client_status* run_context_dist::get_client_status()
    {
        return &f_client_status;
    }

    bool run_context_dist::push_response( int flags )
    {
        size_t t_response_size = reset_buffer( f_response.ByteSize() );
        if( ! f_response.SerializeToArray( f_buffer, t_response_size ) )
            return false;
        try
        {
            f_connection->send_type( f_response_id, flags );
            f_connection->send( f_buffer, t_response_size, flags );
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
        size_t t_response_size = f_buffer_size;
        try
        {
            if( ! verify_message_type( f_response_id, flags ) )
            {
                cerr << "[run_context_dist] message type is not response" << endl;
                return false;
            }
            t_response_size = f_connection->recv_type< size_t >( flags );
            if( t_response_size == 0 ) return false;
            reset_buffer( t_response_size );
            ssize_t recv_ret = f_connection->recv( f_buffer, t_response_size, flags );
            if( recv_ret <= 0 )
            {
                cerr << "[run_context_dist] (response) the connection read length was: " << recv_ret << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "[run_context_dist] a read error occurred while pulling a response: " << e.what() << endl;
            return false;
        }
        return f_response.ParseFromArray( f_buffer, t_response_size );
    }
    response* run_context_dist::get_response()
    {
        return &f_response;
    }

    bool run_context_dist::verify_message_type( run_context_dist::message_id_type a_type, int flags )
    {
        // peek at the message type, without marking that data as read
        if( f_connection->recv_type< message_id_type >( flags | MSG_PEEK ) != a_type )
        {
            return false;
        }
        // if all is well, mark the message-type data as read
        f_connection->recv_type< message_id_type >( flags );
        return true;
    }

}
