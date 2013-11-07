#include "mt_run_context.hh"

#include "mt_exception.hh"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

namespace mantis
{

    run_context::run_context() :
                    f_connection(),
                    f_request(),
                    f_status(),
                    f_response(),
                    f_buffer(NULL)
    {
        f_buffer_size = 0;
    }
    run_context::~run_context()
    {
    }

    void run_context::set_connection( connection* a_connection )
    {
        f_connection = a_connection;
        return;
    }
    connection* run_context::get_connection()
    {
        return f_connection;
    }

    bool run_context::push_request()
    {
        size_t t_request_size = reset_buffer( f_request.ByteSize() );
        cout << "request size to write: " << t_request_size << endl;
        if( ! f_request.SerializeToArray( f_buffer, t_request_size ) )
            return false;
        try
        {
            f_connection->write( (char*)&t_request_size, sizeof( size_t ) );
            f_connection->write( f_buffer, t_request_size );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a request: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool run_context::pull_request()
    {
        size_t t_request_size = f_buffer_size;
        try
        {
            f_connection->read( (char*)&t_request_size, sizeof( size_t ) );
            cout << "request size read: " << t_request_size << endl;
            reset_buffer( t_request_size );
            if( f_connection->read( f_buffer, t_request_size ) == 0 )
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
        cout << f_request.ParseFromArray( f_buffer, t_request_size ) << endl;
        std::string str;
        google::protobuf::TextFormat::PrintToString(f_request, &str);
        cout << str << endl;
        return f_request.ParseFromArray( f_buffer, t_request_size );
    }
    request* run_context::get_request()
    {
        return &f_request;
    }

    bool run_context::push_status()
    {
        size_t t_status_size = reset_buffer( f_status.ByteSize() );
        if( ! f_status.SerializeToArray( f_buffer, t_status_size ) )
            return false;
        try
        {
            f_connection->write( (char*)&t_status_size, sizeof( size_t ) );
            f_connection->write( f_buffer, t_status_size );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a status: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool run_context::pull_status()
    {
        size_t t_status_size = f_buffer_size;
        try
        {
            f_connection->read( (char*)&t_status_size, sizeof( size_t ) );
            reset_buffer( t_status_size );
            if( f_connection->read( f_buffer, t_status_size ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a status: " << e.what() << endl;
            return false;
        }
        return f_status.ParseFromArray( f_buffer, t_status_size );
    }
    status* run_context::get_status()
    {
        return &f_status;
    }

    bool run_context::push_response()
    {
        size_t t_response_size = reset_buffer( f_response.ByteSize() );
        if( ! f_response.SerializeToArray( f_buffer, t_response_size ) )
            return false;
        try
        {
            f_connection->write( (char*)&t_response_size, sizeof( size_t ) );
            f_connection->write( f_buffer, t_response_size );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a response: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool run_context::pull_response()
    {
        size_t t_response_size = f_buffer_size;
        try
        {
            f_connection->read( (char*)&t_response_size, sizeof( size_t ) );
            reset_buffer( t_status_size );
            if( f_connection->read( f_buffer, t_response_size ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a response: " << e.what() << endl;
            return false;
        }
        return f_response.ParseFromArray( f_buffer, t_response_size );
    }
    response* run_context::get_response()
    {
        return &f_response;
    }

    size_t run_context::reset_buffer( size_t a_size )
    {
        if( a_size > f_buffer_size )
        {
            delete [] f_buffer;
            f_buffer_size = a_size;
            f_buffer = new char[ f_buffer_size ];
        }
        ::memset( f_buffer, 0, f_buffer_size );
        // return the requested size
        return a_size;
    }

}
