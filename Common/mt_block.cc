#include "mt_block.hh"

namespace mantis
{
    block::block() :
            f_state( unused ),
            f_acquisition_id( 0 ),
            f_record_id( 0 ),
            f_timestamp( 0 ),
            f_data_size( 0 ),
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
    }

    block::~block()
    {
        if( f_cleanup != NULL ) f_cleanup->delete_memblock();
        delete f_cleanup;
    }

    block::state_t block::get_state() const
    {
        return f_state;
    }
    void block::set_state( state_t a_state )
    {
        f_state = a_state ;
        return;
    }

    bool block::is_unused() const
    {
        return f_state == unused;
    }
    void block::set_unused()
    {
        f_state = unused;
        return;
    }

    bool block::is_acquiring() const
    {
        return f_state == acquiring;
    }
    void block::set_acquiring()
    {
        f_state = acquiring;
        return;
    }

    bool block::is_acquired() const
    {
        return f_state == acquired;
    }
    void block::set_acquired()
    {
        f_state = acquired;
        return;
    }

    bool block::is_processing() const
    {
        return f_state == processing;
    }
    void block::set_processing()
    {
        f_state = processing;
        return;
    }

    bool block::is_writing() const
    {
        return f_state == writing;
    }
    void block::set_writing()
    {
        f_state = writing;
        return;
    }

    bool block::is_written() const
    {
        return f_state == written;
    }
    void block::set_written()
    {
        f_state = written;
        return;
    }

    acquisition_id_type block::get_acquisition_id() const
    {
        return f_acquisition_id;
    }
    void block::set_acquisition_id( acquisition_id_type an_id )
    {
        f_acquisition_id = an_id;
        return;
    }

    record_id_type block::get_record_id() const
    {
        return f_record_id;
    }
    void block::set_record_id( record_id_type an_id )
    {
        f_record_id = an_id;
        return;
    }

    time_nsec_type block::get_timestamp() const
    {
        return f_timestamp;
    }
    void block::set_timestamp( time_nsec_type a_timestamp )
    {
        f_timestamp = a_timestamp;
        return;
    }

    size_t block::get_data_size() const
    {
        return f_data_size;
    }
    void block::set_data_size( size_t a_size )
    {
        f_data_size = a_size;
        return;
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
