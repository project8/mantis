#ifndef MT_RECORD_RECEIVER_HH_
#define MT_RECORD_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "thorax.hh"

namespace mantis
{
    class block;
    class buffer;
    class condition;
    class record_dist;
    class response;
    class server;

    class record_receiver :
        public callable
    {
        public:
            record_receiver( server* a_server, buffer* a_buffer, condition* a_condition );
            virtual ~record_receiver();

            void execute();
            void cancel();
            void finalize( response* a_response );

            size_t get_data_chunk_size();
            void set_data_chunk_size( size_t size );

        private:
            server* f_server;
            buffer* f_buffer;
            condition* f_condition;

            record_id_type f_record_count;
            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            size_t f_data_chunk_size;

            atomic_bool f_canceled;

            bool receive( block* a_block, record_dist* a_dist );

    };

}

#endif
