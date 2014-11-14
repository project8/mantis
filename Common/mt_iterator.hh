#ifndef MT_ITERATOR_HH_
#define MT_ITERATOR_HH_

#include "mt_block.hh"
#include "mt_buffer.hh"
#include "mt_mutex.hh"
#include "mt_exception.hh"
#include "mt_iterator_timer.hh"

namespace mantis
{

    class iterator
    {
        public:
            iterator( buffer* a_buffer, const std::string& a_name = "default" );
            virtual ~iterator();

            const std::string& name() const;

            /// returns the index of the current block in the buffer
            unsigned int index() const;
            /// returns a pointer to the current block
            block* object();

            /// returns a reference to the current block
            block& operator*();
            /// returns a pointer to the current block
            block* operator->();

            /// move to the next block; blocks thread if the next block is locked
            void operator++();
            /// try to move to the next block; fails if the next block's mutex is locked
            bool operator+();
            /// move to the previous block; blocks thread if the next block is locked
            void operator--();
            /// try to move to the previous block;
            bool operator-();

            /// remove iterator from buffer
            void release();

        protected:
            iterator();
            iterator( const iterator& );

            std::string f_name;

            buffer* f_buffer;
            block** f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;

            void increment();
            void decrement();
            unsigned int f_previous_index;
            unsigned int f_current_index;
            unsigned int f_next_index;

            bool f_released;

            IT_TIMER_DECLARE
    };

    /*
    template< typename DataType >
    class typed_iterator : public iterator
    {
        public:
            typed_iterator( buffer* a_buffer );
            virtual ~typed_iterator();

            typed_block< DataType >* typed_object();

            typed_block< DataType >& operator*();
            typed_block< DataType >* operator->();
    };

    template< typename DataType >
    typed_iterator< DataType >::typed_iterator( buffer* a_buffer ) :
            iterator( a_buffer )
    {
        // verify that the blocks are of the right type by checking the first with a dynamic cast
        if( f_blocks[ 0 ] != NULL && dynamic_cast< typed_block< DataType >* >( f_blocks[ 0 ] ) == NULL )
        {
            throw exception() << "buffer contained blocks of the wrong type";
        }
    }

    template< typename DataType >
    typed_iterator< DataType >::~typed_iterator()
    {
    }

    template< typename DataType >
    typed_block< DataType >* typed_iterator< DataType >::typed_object()
    {
        return static_cast< typed_block< DataType >* >( f_blocks[ f_current_index ] );
    }

    template< typename DataType >
    typed_block< DataType >* typed_iterator< DataType >::operator->()
    {
        return static_cast< typed_block< DataType >* >( f_blocks[ f_current_index ] );
    }
    template< typename DataType >
    typed_block< DataType >& typed_iterator< DataType >::operator*()
    {
        return *static_cast< typed_block< DataType >* >( f_blocks[ f_current_index ] );
    }
*/

}

#endif
