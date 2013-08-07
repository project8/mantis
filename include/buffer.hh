#ifndef BUFFER_HH_
#define BUFFER_HH_

#include "iterator.hh"
#include "block.hh"
#include "mutex.hh"

namespace mantis
{

    class iterator;

    class buffer
    {
        public:
            buffer( const unsigned int& a_size );
            virtual ~buffer();

            iterator get_iterator();
            unsigned int get_size();

        private:
            block* f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;
    };

}

#endif
