#ifndef MT_BUFFER_HH_
#define MT_BUFFER_HH_

#include "mt_block.hh"
#include "mt_mutex.hh"

namespace mantis
{

    class buffer
    {
        public:
            friend class iterator;

            buffer( const unsigned int& a_size );
            virtual ~buffer();

            const unsigned int& size() const;

        private:
            block* f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;
    };

}

#endif
