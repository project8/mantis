#define MANTIS_API_EXPORTS

#include "mt_iterator.hh"

#include "logger.hh"

namespace mantis
{
    LOGGER( mtlog, "iterator" );

    iterator::iterator( buffer* a_buffer, const std::string& a_name ) :
            f_name( a_name ),
            f_buffer( a_buffer ),
            f_blocks( a_buffer->f_blocks ),
            f_mutexes( a_buffer->f_mutexes ),
            f_size( a_buffer->f_size ),
            f_previous_index( f_size ),
            f_current_index( 0 ),
            f_next_index( 1 ),
            f_released( false )
    {
        IT_TIMER_INITIALIZE;

        // start out by passing any blocks that are currently locked, then lock the first free block
        IT_TIMER_SET_IGNORE_DECR( (*this) );
        while( f_mutexes[ f_current_index ].trylock() == false )
        {
            decrement();
        }
        IT_TIMER_UNSET_IGNORE_DECR( (*this) );
        LDEBUG( mtlog, "iterator " << f_name << " starting at index " << f_current_index );
    }
    iterator::iterator( const iterator& a_copy )
    {
        f_buffer = a_copy.f_buffer;
        f_blocks = a_copy.f_blocks;
        f_mutexes = a_copy.f_mutexes;
        f_size = a_copy.f_size;
        f_previous_index = a_copy.f_previous_index;
        f_current_index = a_copy.f_current_index;
        f_next_index = a_copy.f_next_index;
        f_released = a_copy.f_released;
    }
    iterator::~iterator()
    {
        if(! f_released ) release();
    }

    const std::string& iterator::name() const
    {
        return f_name;
    }

    unsigned int iterator::index() const
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
        IT_TIMER_INCR_TRY_BEGIN;
        if( f_mutexes[ f_next_index ].trylock() == true )
        {
            IT_TIMER_INCR_LOCKED;
            f_mutexes[ f_current_index ].unlock();
            increment();
            return true;
        }
        else
        {
            IT_TIMER_INCR_TRY_FAIL
            return false;
        }
    }
    void iterator::operator++()
    {
        IT_TIMER_INCR_BEGIN;
        f_mutexes[ f_next_index ].lock();
        IT_TIMER_INCR_LOCKED;
        f_mutexes[ f_current_index ].unlock();
        increment();
        return;
    }

    bool iterator::operator-()
    {
        IT_TIMER_OTHER
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
        IT_TIMER_OTHER
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

    void iterator::release()
    {
        f_mutexes[ f_current_index ].unlock();
        f_buffer = NULL;
        f_blocks = NULL;
        f_mutexes = NULL;
        f_size = 0;
        f_previous_index = 0;
        f_current_index = 0;
        f_next_index = 0;
        f_released = true;
        return;
    }

}
