#ifndef MT_RECORD_RECEIVER_HH_
#define MT_RECORD_RECEIVER_HH_

#include "mt_callable.hh"

#include "mt_atomic.hh"
#include "mt_block.hh"

#include "thorax.hh"

namespace mantis
{
    class buffer;
    class condition;
    class record_dist;
    class response;
    class server;

    template< typename DataType >
    struct block_cleanup_rr : block_cleanup
    {
        block_cleanup_rr( DataType* a_data );
        virtual ~block_cleanup_rr();
        virtual bool delete_data();
        bool f_triggered;
        DataType* f_data;
    };

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
        typed_iterator< DataType > t_it( f_buffer );
        for( unsigned int index = 0; index < f_buffer->size(); index++ )
        {
            block* t_new_block = new typed_block< DataType >();
            *( t_it->handle() ) = new DataType[ f_buffer->record_size() ];
            t_it->set_data_size( f_buffer->record_size() );
            t_new_block->set_cleanup( new block_cleanup_rr< DataType >( t_it->handle() ) );
            f_buffer->set_block( t_it.index(), t_new_block );

            ++t_it;
        }
        return;
    }


    //**********************************
    // Block Cleanup -- Request Receiver
    //**********************************

    template< typename DataType >
    block_cleanup_rr< DataType >::block_cleanup_rr( DataType* a_data ) :
        f_triggered( false ),
        f_data( a_data )
    {}

    template< typename DataType >
    block_cleanup_rr< DataType >::~block_cleanup_rr()
    {}

    template< typename DataType >
    bool block_cleanup_rr< DataType >::delete_data()
    {
        if( f_triggered ) return true;
        delete [] f_data;
        return true;
    }


}

#endif
