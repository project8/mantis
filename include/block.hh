#ifndef BLOCK_HH_
#define BLOCK_HH_

#include "types.hh"

namespace mantis
{

    class block
    {
        private:
            typedef enum
            {
                e_acquiring = 0, e_acquired = 1, e_writing = 2, e_written = 3
            } state_t;

        public:
            block();
            virtual ~block();

            bool is_acquiring() const;
            void set_acquiring();

            bool is_acquired() const;
            void set_acquired();

            bool is_writing() const;
            void set_writing();

            bool is_written() const;
            void set_written();

            const acquisition_id_t& get_acquisition_id() const;
            void set_acquisition_id( const acquisition_id_t& an_id );

            const record_id_t& get_record_id() const;
            void set_record_id( const record_id_t& an_id );

            const timestamp_t& get_timestamp() const;
            void set_timestamp( const timestamp_t& a_timestamp );

            data_t* data();

        private:
            state_t f_state;
            acquisition_id_t f_acquisition_id;
            record_id_t f_record_id;
            timestamp_t f_timestamp;
            data_t* f_data;

    };

}

#endif
