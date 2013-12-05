#ifndef MT_REQUEST_RECEIVER_HH_
#define MT_REQUEST_RECEIVER_HH_

#include "mt_callable.hh"

#include <cstddef>

namespace mantis
{
    class buffer;
    class condition;
    class run_queue;
    class server;

    class request_receiver : public callable
    {
        public:
            request_receiver( server* a_server, run_queue* a_run_queue, condition* a_condition );
            virtual ~request_receiver();

            void execute();
            void cancel();

            size_t get_buffer_size() const;
            void set_buffer_size( size_t size );

            size_t get_record_size() const;
            void set_record_size( size_t size );

            size_t get_data_chunk_size() const;
            void set_data_chunk_size( size_t size );

        private:
            server* f_server;
            run_queue* f_run_queue;
            condition* f_condition;

            size_t f_buffer_size;
            size_t f_record_size;
            size_t f_data_chunk_size;
    };

}

#endif
