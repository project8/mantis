#include "context.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace mantis
{

    context::context() :
            f_connection(),
            f_request(),
            f_status(),
            f_response()
    {
    }
    context::~context()
    {
    }

    void context::set_connection( connection* a_connection )
    {
        f_connection = a_connection;
        return;
    }
    connection* context::get_connection()
    {
        return f_connection;
    }

    void context::push_request()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_request.SerializeToArray( f_buffer, f_buffer_length );
        f_connection->write( f_buffer, f_buffer_length );
        return;
    }
    void context::pull_request()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_connection->read( f_buffer, f_buffer_length );
        f_request.ParseFromArray( f_buffer, f_buffer_length );
        return;
    }
    request* context::get_request()
    {
        return &f_request;
    }

    void context::push_status()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_status.SerializeToArray( f_buffer, f_buffer_length );
        f_connection->write( f_buffer, f_buffer_length );
        return;
    }
    void context::pull_status()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_connection->read( f_buffer, f_buffer_length );
        f_status.ParseFromArray( f_buffer, f_buffer_length );
        return;
    }
    status* context::get_status()
    {
        return &f_status;
    }

    void context::push_response()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_response.SerializeToArray( f_buffer, f_buffer_length );
        f_connection->write( f_buffer, f_buffer_length );
        return;
    }
    void context::pull_response()
    {
        ::memset( f_buffer, 0, f_buffer_length );
        f_connection->read( f_buffer, f_buffer_length );
        f_response.ParseFromArray( f_buffer, f_buffer_length );
        return;
    }
    response* context::get_response()
    {
        return &f_response;
    }

}
