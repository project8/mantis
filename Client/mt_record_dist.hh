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

            block::state_type get_state() const;
            acquisition_id_type get_acquisition_id() const;
            record_id_type get_record_id() const;
            time_nsec_type get_timestamp() const;
            size_t get_data_size() const;
            const data_type* get_data() const;
            bool push_record( const block* a_block, int flags = 0 );
            bool pull_record( block* a_block, int flags = 0 );


        private:
            size_t block_size( const block* a_block ) const;
            bool serialize_block( const block* a_block );
            bool deserialize_block( block* a_block );

            unsigned f_state_offset;
            unsigned f_acquisition_id_offset;
            unsigned f_record_id_offset;
            unsigned f_timestamp_offset;
            unsigned f_data_size_offset;
            unsigned f_data_offset;
    };

}

#endif
