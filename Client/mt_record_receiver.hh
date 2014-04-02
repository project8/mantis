#ifndef MT_RECORD_RECEIVER_HH_
#define MT_RECORD_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"
#include "mt_iterator.hh"

#include "thorax.hh"

namespace mantis
{
    class buffer;
    class condition;
    class record_dist;
    class response;
    class server;

    class block_cleanup_rr;

    class record_receiver :
        public callable
    {
        public:
            record_receiver( server* a_server );
            virtual ~record_receiver();

            bool allocate( buffer* a_buffer, condition* a_condition );

            void execute();
            void cancel();
            void finalize( response* a_response );

            size_t get_data_chunk_size();
            void set_data_chunk_size( size_t size );

            size_t get_data_type_size();
            void set_data_type_size( size_t size );

        private:
            server* f_server;
            buffer* f_buffer;
            condition* f_condition;

            record_id_type f_record_count;
            time_nsec_type f_live_time;
            time_nsec_type f_dead_time;

            size_t f_data_chunk_size;
            size_t f_data_type_size;

            atomic_bool f_canceled;

            bool receive( block* a_block, record_dist* a_dist );

            template< typename DataType >
            void allocate_buffer();

    };

    template< typename DataType >
    void record_receiver::allocate_buffer()
    {
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            block* t_new_block = block::allocate_block< DataType >( f_buffer->record_size() );
            t_new_block->set_cleanup( new block_cleanup_rr( t_new_block->data_bytes() ) );
            f_buffer->set_block( index, t_new_block );
        }
        return;
    }


    class block_cleanup_rr : public block_cleanup
    {
        public:
            block_cleanup_rr( byte_type* a_data );
            virtual ~block_cleanup_rr();
            virtual bool delete_data();
        private:
            bool f_triggered;
            byte_type* f_data;
    };



}

#endif
