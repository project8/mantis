#ifndef MT_BLOCK_HH_
#define MT_BLOCK_HH_

#include "block_header.pb.h"
#include "thorax.hh"

namespace mantis
{

    class block
    {
        public:
            block();
            virtual ~block();

            block_header_state_t get_state() const;
            void set_state( block_header_state_t a_state );

            bool is_acquiring() const;
            void set_acquiring();

            bool is_acquired() const;
            void set_acquired();

            bool is_writing() const;
            void set_writing();

            bool is_written() const;
            void set_written();

            acquisition_id_type get_acquisition_id() const;
            void set_acquisition_id( const acquisition_id_type& an_id );

            record_id_type get_record_id() const;
            void set_record_id( const record_id_type& an_id );

            time_nsec_type get_timestamp() const;
            void set_timestamp( const time_nsec_type& a_timestamp );

            size_t get_data_size() const;
            void set_data_size( const size_t& a_size );

            block_header* header();
            const block_header* header() const;

            data_type* data();
            const data_type* data() const;

            data_type** handle();

        private:
            block_header f_header;
            data_type* f_data;

    };

}

#endif
