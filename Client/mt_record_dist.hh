#ifndef MT_RECORD_DIST_HH_
#define MT_RECORD_DIST_HH_

#include "mt_distribution.hh"

#include "mt_block.hh"

namespace mantis
{

    class record_dist : public distribution
    {
        public:
            record_dist();
            virtual ~record_dist();

            bool push_record( const block* a_block, int flags = 0 );
            bool pull_record( block* a_block, int flags = 0 );

            size_t get_data_chunk_nbytes();
            void set_data_chunk_nbytes( size_t nbytes );

        private:
            bool push_header( const block_header* a_block_header, int flags = 0 );
            bool push_data( const byte_type* a_block_data, int flags = 0 );

            bool pull_header( block_header* a_block_header, int flags = 0 );
            bool pull_data( byte_type* a_block_data, int flags = 0 );

            size_t f_data_chunk_nbytes;
            size_t f_n_full_chunks;
            size_t f_last_data_chunk_nbytes;
    };

}

#endif
