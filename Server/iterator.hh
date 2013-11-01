#ifndef ITERATOR_HH_
#define ITERATOR_HH_

#include "buffer.hh"
#include "mutex.hh"
#include "block.hh"

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

            block* f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;

            void increment();
            void decrement();
            unsigned int f_previous_index;
            unsigned int f_current_index;
            unsigned int f_next_index;
    };

}

#endif
