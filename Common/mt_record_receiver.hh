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
    class server_tcp;

    class block_cleanup_rr;

    class record_receiver :
        public callable
    {
        public:
            record_receiver( server_tcp* a_server );
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
            server_tcp* f_server;
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


    class block_cleanup_rr : public block_cleanup
    {
        public:
            block_cleanup_rr( byte_type* a_memblock );
            virtual ~block_cleanup_rr();
            virtual bool delete_memblock();
        private:
            bool f_triggered;
            byte_type* f_memblock;
    };


    template< typename DataType >
    void record_receiver::allocate_buffer()
    {
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            block* t_new_block = block::allocate_block< DataType >( f_buffer->block_size() );
            t_new_block->set_cleanup( new block_cleanup_rr( t_new_block->memblock_bytes() ) );
            f_buffer->set_block( index, t_new_block );
        }
        return;
    }

}

#endif
