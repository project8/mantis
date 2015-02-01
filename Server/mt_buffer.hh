#ifndef MT_BUFFER_HH_
#define MT_BUFFER_HH_

#include "mt_block.hh"
#include "mt_mutex.hh"

namespace mantis
{

    class MANTIS_API buffer
    {
        public:
            friend class iterator;

            buffer( const unsigned int& a_size, const unsigned int& a_block_size );
            virtual ~buffer();

            const unsigned int& size() const;
            const unsigned int& block_size() const;

            void set_block( unsigned a_index, block* a_block );
            void delete_block( unsigned a_index );

            void print_states();

        private:
            block** f_blocks;
            mutex* f_mutexes;
            unsigned int f_size;
            unsigned int f_block_size;
    };

}

#endif
