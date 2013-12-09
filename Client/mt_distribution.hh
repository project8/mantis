#ifndef MT_DISTRIBUTION_HH_
#define MT_DISTRIBUTION_HH_

#include "mt_connection.hh"

namespace mantis
{

    class distribution
    {
        public:
            distribution();
            virtual ~distribution();

            void set_connection( connection* a_connection );
            connection* get_connection();

        protected:
            size_t reset_buffer_in( size_t a_size );
            size_t reset_buffer_out( size_t a_size );

            connection* f_connection;

            size_t f_buffer_in_size;
            char* f_buffer_in;

            size_t f_buffer_out_size;
            char* f_buffer_out;
    };

}

#endif
