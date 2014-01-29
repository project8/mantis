#ifndef MT_BLOCK_HH_
#define MT_BLOCK_HH_

#include "block_header.pb.h"
#include "thorax.hh"

namespace mantis
{
    struct block_cleanup
    {
            virtual ~block_cleanup() {};
            virtual bool delete_data() = 0;
    };

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

            virtual char* data_bytes() = 0;
            virtual const char* data_bytes() const = 0;

            void set_cleanup( block_cleanup* a_cleanup );

        protected:
            block_header f_header;

            block_cleanup* f_cleanup;

    };

    class empty_block : public block
    {
        public:
            empty_block();
            virtual ~empty_block();

            virtual size_t get_data_nbytes() const;

            virtual char* data_bytes();
            virtual const char* data_bytes() const;
    };

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

            virtual char* data_bytes();
            virtual const char* data_bytes() const;

        private:
            DataType* f_data;

    };

    template< typename DataType >
    typed_block< DataType >::typed_block() :
            typed_block(),
            f_data( NULL )
    {
    }

    template< typename DataType >
    typed_block< DataType >::~typed_block()
    {
    }

    template< typename DataType >
    size_t typed_block< DataType >::get_data_nbytes() const
    {
        return sizeof( DataType ) * f_header->data_size();
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
    char* typed_block< DataType >::data_bytes()
    {
        return ( char* )f_data;
    }

    template< typename DataType >
    const char* typed_block< DataType >::data_bytes() const
    {
        return ( const char* )f_data;
    }


}

#endif
