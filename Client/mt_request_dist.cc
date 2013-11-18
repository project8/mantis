#include "mt_request_dist.hh"

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

    request_dist::request_dist() :
                    f_request(),
                    f_status(),
                    f_client_status(),
                    f_response()
    {
    }
    request_dist::~request_dist()
    {
    }

    bool request_dist::push_request( int flags )
    {
        size_t t_request_size = reset_buffer( f_request.ByteSize() );
        cout << "request size to write: " << t_request_size << endl;
        if( ! f_request.SerializeToArray( f_buffer, t_request_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_request_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a request: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool request_dist::pull_request( int flags )
    {
        size_t t_request_size = f_buffer_size;
        try
        {
            t_request_size = f_connection->recv_size( flags );
            if( t_request_size == 0 ) return false;
            reset_buffer( t_request_size );
            if( f_connection->recv( f_buffer, t_request_size, flags ) == 0 )
            {
                cout << "connection read length was 0" << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a request: " << e.what() << endl;
            return false;
        }
        return f_request.ParseFromArray( f_buffer, t_request_size );
    }
    request* request_dist::get_request()
    {
        return &f_request;
    }

    bool request_dist::push_status( int flags )
    {
        size_t t_status_size = reset_buffer( f_status.ByteSize() );
        if( ! f_status.SerializeToArray( f_buffer, t_status_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_status_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a status: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool request_dist::pull_status( int flags )
    {
        size_t t_status_size = f_buffer_size;
        try
        {
            t_status_size = f_connection->recv_size( flags );
            if( t_status_size == 0 ) return false;
            reset_buffer( t_status_size );
            if( f_connection->recv( f_buffer, t_status_size, flags ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a status: " << e.what() << endl;
            return false;
        }
        return f_status.ParseFromArray( f_buffer, t_status_size );
    }
    status* request_dist::get_status()
    {
        return &f_status;
    }

    bool request_dist::push_client_status( int flags )
    {
        size_t t_client_status_size = reset_buffer( f_client_status.ByteSize() );
        if( ! f_client_status.SerializeToArray( f_buffer, t_client_status_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_client_status_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a client_status: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool request_dist::pull_client_status( int flags )
    {
        size_t t_client_status_size = f_buffer_size;
        try
        {
            t_client_status_size = f_connection->recv_size( flags );
            if( t_client_status_size == 0 ) return false;
            reset_buffer( t_client_status_size );
            if( f_connection->recv( f_buffer, t_client_status_size, flags ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a client_status: " << e.what() << endl;
            return false;
        }
        return f_client_status.ParseFromArray( f_buffer, t_client_status_size );
    }
    client_status* request_dist::get_client_status()
    {
        return &f_client_status;
    }

    bool request_dist::push_response( int flags )
    {
        size_t t_response_size = reset_buffer( f_response.ByteSize() );
        if( ! f_response.SerializeToArray( f_buffer, t_response_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_response_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a response: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool request_dist::pull_response( int flags )
    {
        size_t t_response_size = f_buffer_size;
        try
        {
            t_response_size = f_connection->recv_size( flags );
            if( t_response_size == 0 ) return false;
            reset_buffer( t_response_size );
            if( f_connection->recv( f_buffer, t_response_size, flags ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a response: " << e.what() << endl;
            return false;
        }
        return f_response.ParseFromArray( f_buffer, t_response_size );
    }
    response* request_dist::get_response()
    {
        return &f_response;
    }

}
