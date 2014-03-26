#include "mt_iterator.hh"

namespace mantis
{

    iterator::iterator( buffer* a_buffer ) :
            f_blocks( a_buffer->f_blocks ),
            f_mutexes( a_buffer->f_mutexes ),
            f_size( a_buffer->f_size ),
            f_previous_index( f_size ),
            f_current_index( 0 ),
            f_next_index( 1 )
    {
        // start out by passing any blocks that are currently locked, then lock the first free block
        while( f_mutexes[ f_current_index ].trylock() == false )
        {
            increment();
        }
    }
    iterator::iterator( const iterator& a_copy )
    {
        f_blocks = a_copy.f_blocks;
        f_mutexes = a_copy.f_mutexes;
        f_size = a_copy.f_size;
        f_previous_index = a_copy.f_previous_index;
        f_current_index = a_copy.f_current_index;
        f_next_index = a_copy.f_next_index;
    }
    iterator::~iterator()
    {
        f_mutexes[ f_current_index ].unlock();
    }

    unsigned int iterator::index()
    {
        return f_current_index;
    }
    block* iterator::object()
    {
        return f_blocks[ f_current_index ];
    }

    block* iterator::operator->()
    {
        return f_blocks[ f_current_index ];
    }
    block& iterator::operator*()
    {
        return *f_blocks[ f_current_index ];
    }

    bool iterator::operator+()
    {
        if( f_mutexes[ f_next_index ].trylock() == true )
        {
            f_mutexes[ f_current_index ].unlock();
            increment();
            return true;
        }
        else
        {
            return false;
        }
    }
    void iterator::operator++()
    {
        f_mutexes[ f_next_index ].lock();
        f_mutexes[ f_current_index ].unlock();
        increment();
        return;
    }

    bool iterator::operator-()
    {
        if( f_mutexes[ f_previous_index ].trylock() == true )
        {
            f_mutexes[ f_current_index ].unlock();
            decrement();
            return true;
        }
        else
        {
            return false;
        }
    }
    void iterator::operator--()
    {
        f_mutexes[ f_previous_index ].lock();
        f_mutexes[ f_current_index ].unlock();
        decrement();
        return;
    }

    void iterator::increment()
    {
        f_previous_index++;
        if( f_previous_index == f_size )
        {
            f_previous_index = 0;
        }
        f_current_index++;
        if( f_current_index == f_size )
        {
            f_current_index = 0;
        }
        f_next_index++;
        if( f_next_index == f_size )
        {
            f_next_index = 0;
        }
        return;
    }
    void iterator::decrement()
    {
        if( f_previous_index == 0 )
        {
            f_previous_index = f_size;
        }
        f_previous_index--;
        if( f_current_index == 0 )
        {
            f_current_index = f_size;
        }
        f_current_index--;
        if( f_next_index == 0 )
        {
            f_next_index = f_size;
        }
        f_next_index--;
        return;
    }

}
