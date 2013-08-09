#include "context.hh"

#include <string>

namespace mantis
{

    context::context( connection* a_connection ) :
            f_connection( a_connection ),
            f_request(),
            f_status(),
            f_response()
    {
    }

    context::~context()
    {
        delete f_connection;
    }

    void context::push_request()
    {
        std::string t_message;
        f_request.SerializePartialToString( &t_message );
        f_connection->write( t_message );
        return;
    }
    void context::pull_request()
    {
        std::string t_message;
        f_connection->read( t_message );
        f_request.ParseFromString( t_message );
        return;
    }
    request* context::get_request()
    {
        return &f_request;
    }

    void context::push_status()
    {
        std::string t_message;
        f_status.SerializePartialToString( &t_message );
        f_connection->write( t_message );
        return;
    }
    void context::pull_status()
    {
        std::string t_message;
        f_connection->read( t_message );
        f_status.ParseFromString( t_message );
        return;
    }
    status* context::get_status()
    {
        return &f_status;
    }

    void context::push_response()
    {
        std::string t_message;
        f_response.SerializePartialToString( &t_message );
        f_connection->write( t_message );
        return;
    }
    void context::pull_response()
    {
        std::string t_message;
        f_connection->read( t_message );
        f_response.ParseFromString( t_message );
        return;
    }
    response* context::get_response()
    {
        return &f_response;
    }

}
