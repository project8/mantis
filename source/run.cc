#include "run.hh"

#include <string>

namespace mantis
{

    run::run( connection* a_connection ) :
            f_connection( a_connection ),
            f_request(),
            f_response()
    {
    }

    run::~run()
    {
    }

    void run::push_request()
    {
        std::string t_message;
        f_request.SerializePartialToString( &t_message );
        f_connection->write( t_message );
        return;
    }
    void run::pull_request()
    {
        std::string t_message;
        f_connection->read( t_message );
        f_request.ParseFromString( t_message );
        return;
    }
    request& run::get_request()
    {
        return f_request;
    }

    void run::push_response()
    {
        std::string t_message;
        f_response.SerializeToString( &t_message );
        f_connection->write( t_message );
        return;
    }
    void run::pull_response()
    {
        std::string t_message;
        f_connection->read( t_message );
        f_response.ParseFromString( t_message );
        return;
    }
    response& run::get_response()
    {
        return f_response;
    }

}
