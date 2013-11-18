#include "mt_buffer.hh"

namespace mantis
{

    buffer::buffer( const unsigned int& a_size, const unsigned int& a_record_size ) :
            f_blocks( NULL ),
            f_mutexes( NULL ),
            f_size( a_size ),
            f_record_size( a_record_size )
    {
        f_blocks = new block[f_size];
        f_mutexes = new mutex[f_size];
    }

    buffer::~buffer()
    {
        delete [] f_blocks;
        delete [] f_mutexes;
    }

    const unsigned int& buffer::size() const
    {
        return f_size;
    }

    const unsigned int& buffer::record_size() const
    {
        return f_record_size;
    }

}
