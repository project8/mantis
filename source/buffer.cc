#include "buffer.hh"

namespace mantis
{

    buffer::buffer( const unsigned int& a_size ) :
            f_blocks( NULL ),
            f_mutexes( NULL ),
            f_size( a_size )
    {
        f_blocks = new block[f_size];
        f_mutexes = new mutex[f_size];
    }

    buffer::~buffer()
    {
        delete [] f_blocks;
        delete [] f_mutexes;
    }

    iterator buffer::get_iterator()
    {
        return iterator( f_blocks, f_mutexes, f_size );
    }
    unsigned int buffer::get_size()
    {
        return f_size;
    }

}
