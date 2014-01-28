#include "mt_buffer.hh"

namespace mantis
{

    buffer::buffer( const unsigned int& a_size, const unsigned int& a_record_size ) :
            f_blocks( NULL ),
            f_mutexes( NULL ),
            f_size( a_size ),
            f_record_size( a_record_size )
    {
        f_blocks = new block*[f_size];
        for( unsigned t_index = 0; t_index < f_size; ++t_index )
        {
            f_blocks[ t_index ] = NULL;
        }
        f_mutexes = new mutex[f_size];
    }

    buffer::~buffer()
    {
        for( unsigned t_index = 0; t_index < f_size; ++t_index )
        {
            delete_block( t_index );
        }
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

    void buffer::set_block( unsigned a_index, block* a_block )
    {
        f_mutexes[ a_index ].lock();
        delete f_blocks[ a_index ];
        f_blocks[ a_index ] = a_block;
        f_mutexes[ a_index ].unlock();
        return;
    }

    void buffer::delete_block( unsigned a_index )
    {
        set_block( a_index, NULL );
        return;
    }

}
