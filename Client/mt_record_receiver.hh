#ifndef MT_RECORD_RECEIVER_HH_
#define MT_RECORD_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_server.hh"
#include "mt_buffer.hh"
#include "mt_condition.hh"

namespace mantis
{

    class record_receiver :
        public callable
    {
        public:
            record_receiver( server* a_server, buffer* a_buffer, condition* a_condition );
            virtual ~record_receiver();

            void execute();
            void finalize( response* a_response );

            size_t get_data_chunk_size();
            void set_data_chunk_size( size_t size );

        private:
            server* f_server;
            buffer* f_buffer;
            condition* f_condition;

            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            size_t f_data_chunk_size;

            bool receive( block* a_block, record_dist* a_dist );

    };

}

#endif
