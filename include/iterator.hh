#ifndef ITERATOR_HH_
#define ITERATOR_HH_

#include "mutex.hh"
#include "block.hh"

namespace mantis
{

    class iterator
    {
        public:
            iterator( block* a_block_array, mutex* a_mutex_array, const unsigned int& a_size );
            virtual ~iterator();

            unsigned int index();
            block& operator*();
            block* operator->();

            void operator++();
            bool operator+();
            void operator--();
            bool operator-();

        private:
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
