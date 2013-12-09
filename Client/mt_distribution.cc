#include "mt_distribution.hh"

#include "mt_exception.hh"

#include <cstring>

namespace mantis
{

    distribution::distribution() :
            f_connection( NULL ),
            f_buffer_in_size( 0 ),
            f_buffer_in( NULL ),
            f_buffer_out_size( 0 ),
            f_buffer_out( NULL )
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

    size_t distribution::reset_buffer_in( size_t a_size )
    {
        if( a_size > f_buffer_in_size )
        {
            delete [] f_buffer_in;
            f_buffer_in_size = a_size;
            f_buffer_in = new char[ f_buffer_in_size ];
        }
        ::memset( f_buffer_in, 0, f_buffer_in_size );
        // return the requested size
        return a_size;
    }

    size_t distribution::reset_buffer_out( size_t a_size )
    {
        if( a_size > f_buffer_out_size )
        {
            delete [] f_buffer_out;
            f_buffer_out_size = a_size;
            f_buffer_out = new char[ f_buffer_out_size ];
        }
        ::memset( f_buffer_out, 0, f_buffer_out_size );
        // return the requested size
        return a_size;
    }

}
