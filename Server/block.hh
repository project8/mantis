#ifndef BLOCK_HH_
#define BLOCK_HH_

#include "thorax.hh"

namespace mantis
{

    class block
    {
        private:
            typedef enum
            {
                e_acquiring = 0, e_acquired = 1, e_writing = 2, e_written = 3
            } state_type;

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

            const acquisition_id_type& get_acquisition_id() const;
            void set_acquisition_id( const acquisition_id_type& an_id );

            const record_id_type& get_record_id() const;
            void set_record_id( const record_id_type& an_id );

            const time_nsec_type& get_timestamp() const;
            void set_timestamp( const time_nsec_type& a_timestamp );

            data_type* data();
            data_type** handle();

        private:
            state_type f_state;
            acquisition_id_type f_acquisition_id;
            record_id_type f_record_id;
            time_nsec_type f_timestamp;
            data_type* f_data;

    };

}

#endif
