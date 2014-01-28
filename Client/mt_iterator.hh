#ifndef MT_ITERATOR_HH_
#define MT_ITERATOR_HH_

#include "mt_buffer.hh"
#include "mt_mutex.hh"
#include "mt_block.hh"
#include "mt_exception.hh"

namespace mantis
{

    class iterator
    {
        public:
            iterator( buffer* a_buffer );
            virtual ~iterator();

            unsigned int index();
            block* object();

            block& operator*();
            block* operator->();

            void operator++();
            bool operator+();
            void operator--();
            bool operator-();

        private:
            iterator();
            iterator( const iterator& );

            block** f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;

            void increment();
            void decrement();
            unsigned int f_previous_index;
            unsigned int f_current_index;
            unsigned int f_next_index;
    };

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
    typed_iterator< DataType >::typed_iterator( buffer* a_buffer )
    {
        // verify that the blocks are of the right type by checking the first with a dynamic cast
        if( dynamic_cast< typed_block< DataType >* >( f_blocks[ 0 ] ) == NULL )
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
        return f_blocks[ f_current_index ];
    }

    typed_block< DataType >* typed_iterator< DataType >::operator->()
    {
        return f_blocks[ f_current_index ];
    }
    typed_block< DataType >& typed_iterator< DataType >::operator*()
    {
        return *f_blocks[ f_current_index ];
    }


}

#endif
