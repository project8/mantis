#ifndef MT_BLOCK_HH_
#define MT_BLOCK_HH_

#include "mt_constants.hh"

#include "thorax.hh"

namespace mantis
{
    /*!
     @class block
     @author D. Furse, N. Oblath

     @brief Single-record data class for mantis buffers
    */

    /*!
     @class block_cleanup
     @author N. Oblath

     @brief Memory de-allocation class
    */

    /*!
     @class block_view
     @author N. Oblath

     @brief Convenience class giving access to the data array cast to a different type
    */


    class block_cleanup;

    //**************************************************
    // block
    //**************************************************

    class MANTIS_API block
    {
        public:
            //state of block
            enum state_t
            {
                unused = 0,
                acquiring = 1,
                acquired = 2,
                processing = 3,
                writing = 4,
                written = 5
            };

        public:
            block();
            virtual ~block();

            template< typename DataType >
            static block* allocate_block( unsigned a_data_size, unsigned a_prefix_size = 0, unsigned a_postfix_size = 0 );

            state_t get_state() const;
            void set_state( state_t a_state );

            bool is_unused() const;
            void set_unused();

            bool is_acquiring() const;
            void set_acquiring();

            bool is_acquired() const;
            void set_acquired();

            bool is_processing() const;
            void set_processing();

            bool is_writing() const;
            void set_writing();

            bool is_written() const;
            void set_written();

            acquisition_id_type get_acquisition_id() const;
            void set_acquisition_id( acquisition_id_type an_id );

            record_id_type get_record_id() const;
            void set_record_id( record_id_type an_id );

            time_nsec_type get_timestamp() const;
            void set_timestamp( time_nsec_type a_timestamp );

            size_t get_data_size() const;
            void set_data_size( size_t a_size );

            byte_type* memblock_bytes();
            const byte_type* memblock_bytes() const;

            size_t get_memblock_nbytes() const;
            void set_memblock_nbytes( size_t a_nbytes );

            byte_type* prefix_bytes();
            const byte_type* prefix_bytes() const;

            size_t get_prefix_nbytes() const;
            void set_prefix_nbytes( size_t a_nbytes );

            byte_type* data_bytes();
            const byte_type* data_bytes() const;

            size_t get_data_nbytes() const;
            void set_data_nbytes( size_t a_nbytes );

            byte_type* postfix_bytes();
            const byte_type* postfix_bytes() const;

            size_t get_postfix_nbytes() const;
            void set_postfix_nbytes( size_t a_nbytes );

            byte_type** handle();

            void set_cleanup( block_cleanup* a_cleanup );

        protected:
            state_t f_state;
            acquisition_id_type f_acquisition_id;
            record_id_type f_record_id;
            time_nsec_type f_timestamp;
            size_t f_data_size;

            byte_type* f_memblock_bytes;
            size_t f_memblock_nbytes;

            byte_type* f_prefix_bytes;
            size_t f_prefix_nbytes;

            byte_type* f_data_bytes;
            size_t f_data_nbytes;

            byte_type* f_postfix_bytes;
            size_t f_postfix_nbytes;

            block_cleanup* f_cleanup;
    };

    template< typename DataType >
    block* block::allocate_block( unsigned a_data_size, unsigned a_prefix_size, unsigned a_postfix_size )
    {
        size_t t_data_type_size = sizeof( DataType );

        block* t_new_block = new block();
        unsigned t_total_size = a_prefix_size + a_data_size + a_postfix_size;

        t_new_block->f_memblock_nbytes = t_total_size * t_data_type_size;
        t_new_block->f_memblock_bytes = new byte_type [ t_total_size ];

        t_new_block->f_prefix_nbytes = a_prefix_size * t_data_type_size;
        t_new_block->f_prefix_bytes = t_new_block->f_memblock_bytes;

        t_new_block->set_data_size( a_data_size );
        t_new_block->f_data_nbytes = a_data_size * t_data_type_size;
        t_new_block->f_data_bytes = t_new_block->f_prefix_bytes + t_new_block->f_prefix_nbytes;

        t_new_block->f_postfix_nbytes = a_postfix_size * t_data_type_size;
        t_new_block->f_postfix_bytes = t_new_block->f_data_bytes + t_new_block->f_data_nbytes;

        return t_new_block;
    }


    //**************************************************
    // block_cleanup
    //**************************************************

    class MANTIS_API block_cleanup
    {
        public:
            block_cleanup() {}
            virtual ~block_cleanup() {}
            virtual bool delete_memblock() = 0;
    };


    //**************************************************
    // block_view< DataType >
    //**************************************************

    template< typename DataType >
    class block_view
    {
        public:
            typedef DataType data_type;

        public:
            block_view( block* a_block = NULL );
            virtual ~block_view();

            void set_viewed( block* a_block );

            size_t get_data_view_size() const;
            data_type* data_view() const;

        private:
            block* f_block;
            DataType* f_data_view;
            size_t f_view_size;
    };

    template< typename DataType >
    block_view< DataType >::block_view( block* a_block ) :
            f_block( NULL ),
            f_data_view( NULL ),
            f_view_size( 0 )
    {
        if( a_block != NULL ) set_viewed( a_block );
    }

    template< typename DataType >
    block_view< DataType >::~block_view()
    {
    }

    template< typename DataType >
    void block_view< DataType >::set_viewed( block* a_block )
    {
        f_block = a_block;
        f_data_view = reinterpret_cast< DataType* >( a_block->data_bytes() );
        f_view_size = a_block->get_data_size() / sizeof( DataType );
        return;
    }

    template< typename DataType >
    size_t block_view< DataType >::get_data_view_size() const
    {
        return f_view_size;
    }

    template< typename DataType >
    DataType* block_view< DataType >::data_view() const
    {
        return f_data_view;
    }


}

#endif
