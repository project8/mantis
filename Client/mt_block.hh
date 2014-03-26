#ifndef MT_BLOCK_HH_
#define MT_BLOCK_HH_

#include "block_header.pb.h"
#include "thorax.hh"

namespace mantis
{
    /*!
     @class block
     @author D. Furse, N. Oblath

     @brief Single-record data class for mantis buffers; this base class provides the byte-type access functions
    */

    /*!
     @class empty_block
     @author N. Oblath

     @brief Block with no data
    */

    /*!
     @class block_cleanup
     @author N. Oblath

     @brief Memory de-allocation class
    */

    /*!
     @class typed_block
     @author N. Oblath

     @brief Single-record data class for mantis buffers; this templated derived class contains the actual data array
    */

    /*!
     @class block_view
     @author N. Oblath

     @brief Convenience class giving access to the data array cast to a different type
    */



    //**************************************************
    // block
    //**************************************************

    class block
    {
        public:
            block();
            virtual ~block();

            block_header_state_t get_state() const;
            void set_state( block_header_state_t a_state );

            bool is_acquiring() const;
            void set_acquiring();

            bool is_acquired() const;
            void set_acquired();

            bool is_writing() const;
            void set_writing();

            bool is_written() const;
            void set_written();

            acquisition_id_type get_acquisition_id() const;
            void set_acquisition_id( const acquisition_id_type& an_id );

            record_id_type get_record_id() const;
            void set_record_id( const record_id_type& an_id );

            time_nsec_type get_timestamp() const;
            void set_timestamp( const time_nsec_type& a_timestamp );

            size_t get_data_size() const;
            void set_data_size( const size_t& a_size );

            virtual size_t get_data_nbytes() const = 0;

            block_header* header();
            const block_header* header() const;

            virtual byte_type* data_bytes() = 0;
            virtual const byte_type* data_bytes() const = 0;

        protected:
            block_header f_header;

    };


    //**************************************************
    // empty_block
    //**************************************************

    class empty_block : public block
    {
        public:
            empty_block();
            virtual ~empty_block();

            virtual size_t get_data_nbytes() const;

            virtual byte_type* data_bytes();
            virtual const byte_type* data_bytes() const;
    };


    //**************************************************
    // block_cleanup
    //**************************************************

    class block_cleanup
    {
        public:
            block_cleanup() {}
            virtual ~block_cleanup() {}
            virtual bool delete_data() = 0;
    };


    //**************************************************
    // typed_block< DataType >
    //**************************************************

    template< typename DataType >
    class typed_block : public block
    {
        public:
            typed_block();
            virtual ~typed_block();

            virtual size_t get_data_nbytes() const;

            DataType* data();
            const DataType* data() const;

            DataType** handle();

            virtual byte_type* data_bytes();
            virtual const byte_type* data_bytes() const;

            void set_cleanup( block_cleanup* a_cleanup );

        private:
            DataType* f_data;

            block_cleanup* f_cleanup;
    };

    template< typename DataType >
    typed_block< DataType >::typed_block() :
            block(),
            f_data( NULL ),
            f_cleanup( NULL )
    {
    }

    template< typename DataType >
    typed_block< DataType >::~typed_block()
    {
        if( f_cleanup != NULL ) f_cleanup->delete_data();
        delete f_cleanup;
    }

    template< typename DataType >
    size_t typed_block< DataType >::get_data_nbytes() const
    {
        return sizeof( DataType ) * f_header.data_size();
    }

    template< typename DataType >
    DataType* typed_block< DataType >::data()
    {
        return f_data;
    }

    template< typename DataType >
    const DataType* typed_block< DataType >::data() const
    {
        return f_data;
    }

    template< typename DataType >
    DataType** typed_block< DataType >::handle()
    {
        return &f_data;
    }

    template< typename DataType >
    byte_type* typed_block< DataType >::data_bytes()
    {
        return static_cast< byte_type* >( f_data );
    }

    template< typename DataType >
    const byte_type* typed_block< DataType >::data_bytes() const
    {
        return ( const byte_type* )f_data;
    }

    template< typename DataType >
    void typed_block< DataType >::set_cleanup( block_cleanup* a_cleanup )
    {
        delete f_cleanup;
        f_cleanup = a_cleanup;
        return;
    }


    //**************************************************
    // block_view< DataType >
    //**************************************************

    template< typename DataType >
    class block_view : public block
    {
        public:
            typedef DataType data_type;

        public:
            block_view( block* a_block );
            virtual ~block_view();

            data_type* data_view() const;

        private:
            block* f_block;
            DataType* f_data_view;
    };

    template< typename DataType >
    block_view< DataType >::block_view( block* a_block ) :
            f_block( a_block ),
            f_data_view( static_cast< DataType* >( a_block->data_bytes() ) )
    {
    }

    template< typename DataType >
    DataType* block_view< DataType >::data_view() const
    {
        return f_data_view;
    }


}

#endif
