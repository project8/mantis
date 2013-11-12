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
            const data_type* get_data() const;
            bool push_record( const block* a_block );
            bool pull_record();


        private:
            unsigned f_state_offset;
            unsigned f_acquisition_id_offset;
            unsigned f_record_id_offset;
            unsigned f_timestamp_offset;
            unsigned f_data_offset;
    };

}

#endif
