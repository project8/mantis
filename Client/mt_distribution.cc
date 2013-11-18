#include "mt_distribution.hh"

#include "mt_exception.hh"

#include <cstring>

namespace mantis
{

    distribution::distribution() :
                    f_connection(),
                    f_buffer_size(0),
                    f_buffer(NULL)
    {
    }
    distribution::~distribution()
    {
    }

    void distribution::set_connection( connection* a_connection )
    {
        f_connection = a_connection;
        return;
    }
    connection* distribution::get_connection()
    {
        return f_connection;
    }

    size_t distribution::reset_buffer( size_t a_size )
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
