#include "mt_block.hh"

namespace mantis
{
    block::block() :
            f_header(),
            f_memblock_bytes( NULL ),
            f_memblock_nbytes( 0 ),
            f_prefix_bytes( NULL ),
            f_prefix_nbytes( 0 ),
            f_data_bytes( NULL ),
            f_data_nbytes( 0 ),
            f_postfix_bytes( NULL ),
            f_postfix_nbytes( 0 ),
            f_cleanup( NULL )
    {
        f_header.set_state( block_header_state_t_unused );
        f_header.set_acquisition_id( 0 );
        f_header.set_record_id( 0 );
        f_header.set_timestamp( 0 );
        f_header.set_data_size( 0 );
    }

    block::~block()
    {
        if( f_cleanup != NULL ) f_cleanup->delete_memblock();
        delete f_cleanup;
    }

    block_header_state_t block::get_state() const
    {
        return f_header.state();
    }
    void block::set_state( block_header_state_t a_state )
    {
        f_header.set_state( a_state );
        return;
    }

    bool block::is_unused() const
    {
        return f_header.state() == block_header_state_t_unused;
    }
    void block::set_unused()
    {
        f_header.set_state( block_header_state_t_unused );
        return;
    }

    bool block::is_acquiring() const
    {
        return f_header.state() == block_header_state_t_acquiring;
    }
    void block::set_acquiring()
    {
        f_header.set_state( block_header_state_t_acquiring );
        return;
    }

    bool block::is_acquired() const
    {
        return f_header.state() == block_header_state_t_acquired;
    }
    void block::set_acquired()
    {
        f_header.set_state( block_header_state_t_acquired );
        return;
    }

    bool block::is_processing() const
    {
        return f_header.state() == block_header_state_t_processing;
    }
    void block::set_processing()
    {
        f_header.set_state( block_header_state_t_processing );
        return;
    }

    bool block::is_writing() const
    {
        return f_header.state() == block_header_state_t_writing;
    }
    void block::set_writing()
    {
        f_header.set_state( block_header_state_t_writing );
        return;
    }

    bool block::is_written() const
    {
        return f_header.state() == block_header_state_t_written;
    }
    void block::set_written()
    {
        f_header.set_state( block_header_state_t_written );
        return;
    }

    acquisition_id_type block::get_acquisition_id() const
    {
        return f_header.acquisition_id();
    }
    void block::set_acquisition_id( acquisition_id_type an_id )
    {
        f_header.set_acquisition_id( an_id );
        return;
    }

    record_id_type block::get_record_id() const
    {
        return f_header.record_id();
    }
    void block::set_record_id( record_id_type an_id )
    {
        f_header.set_record_id( an_id );
        return;
    }

    time_nsec_type block::get_timestamp() const
    {
        return f_header.timestamp();
    }
    void block::set_timestamp( time_nsec_type a_timestamp )
    {
        f_header.set_timestamp( a_timestamp );
        return;
    }

    size_t block::get_data_size() const
    {
        return f_header.data_size();
    }
    void block::set_data_size( size_t a_size )
    {
        f_header.set_data_size( a_size );
        return;
    }

    block_header* block::header()
    {
        return &f_header;
    }

    const block_header* block::header() const
    {
        return &f_header;
    }

    byte_type* block::memblock_bytes()
    {
        return f_memblock_bytes;
    }

    const byte_type* block::memblock_bytes() const
    {
        return f_memblock_bytes;
    }

    size_t block::get_memblock_nbytes() const
    {
        return f_memblock_nbytes;
    }

    void block::set_memblock_nbytes( size_t a_nbytes )
    {
        f_memblock_nbytes = a_nbytes;
        return;
    }

    byte_type* block::prefix_bytes()
    {
        return f_prefix_bytes;
    }

    const byte_type* block::prefix_bytes() const
    {
        return f_prefix_bytes;
    }

    size_t block::get_prefix_nbytes() const
    {
        return f_prefix_nbytes;
    }

    void block::set_prefix_nbytes( size_t a_nbytes )
    {
        f_prefix_nbytes = a_nbytes;
        return;
    }

    byte_type* block::data_bytes()
    {
        return f_data_bytes;
    }

    const byte_type* block::data_bytes() const
    {
        return f_data_bytes;
    }

    size_t block::get_data_nbytes() const
    {
        return f_data_nbytes;
    }

    void block::set_data_nbytes( size_t a_nbytes )
    {
        f_data_nbytes = a_nbytes;
        return;
    }

    byte_type* block::postfix_bytes()
    {
        return f_postfix_bytes;
    }

    const byte_type* block::postfix_bytes() const
    {
        return f_postfix_bytes;
    }

    size_t block::get_postfix_nbytes() const
    {
        return f_postfix_nbytes;
    }

    void block::set_postfix_nbytes( size_t a_nbytes )
    {
        f_postfix_nbytes = a_nbytes;
        return;
    }

    byte_type** block::handle()
    {
        return &f_memblock_bytes;
    }

    void block::set_cleanup( block_cleanup* a_cleanup )
    {
        delete f_cleanup;
        f_cleanup = a_cleanup;
        return;
    }
}
