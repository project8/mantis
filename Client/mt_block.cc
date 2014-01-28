#include "mt_block.hh"

namespace mantis
{

    block::block() :
            f_header(),
            f_cleanup( NULL )
    {
        f_header.set_state( block_header_state_t_written );
        f_header.set_acquisition_id( 0 );
        f_header.set_record_id( 0 );
        f_header.set_timestamp( 0 );
        f_header.set_data_size( 0 );
    }

    block::~block()
    {
        if( f_cleanup != NULL ) f_cleanup->delete_data();
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

    bool block::is_acquiring() const
    {
        if( f_header.state() == block_header_state_t_acquiring )
        {
            return true;
        }
        return false;
    }
    void block::set_acquiring()
    {
        f_header.set_state( block_header_state_t_acquiring );
        return;
    }

    bool block::is_acquired() const
    {
        if( f_header.state() == block_header_state_t_acquired )
        {
            return true;
        }
        return false;
    }
    void block::set_acquired()
    {
        f_header.set_state( block_header_state_t_acquired );
        return;
    }

    bool block::is_writing() const
    {
        if( f_header.state() == block_header_state_t_writing )
        {
            return true;
        }
        return false;
    }
    void block::set_writing()
    {
        f_header.set_state( block_header_state_t_writing );
        return;
    }

    bool block::is_written() const
    {
        if( f_header.state() == block_header_state_t_written )
        {
            return true;
        }
        return false;
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
    void block::set_acquisition_id( const acquisition_id_type& an_id )
    {
        f_header.set_acquisition_id( an_id );
        return;
    }

    record_id_type block::get_record_id() const
    {
        return f_header.record_id();
    }
    void block::set_record_id( const record_id_type& an_id )
    {
        f_header.set_record_id( an_id );
        return;
    }

    time_nsec_type block::get_timestamp() const
    {
        return f_header.timestamp();
    }
    void block::set_timestamp( const time_nsec_type& a_timestamp )
    {
        f_header.set_timestamp( a_timestamp );
        return;
    }

    size_t block::get_data_size() const
    {
        return f_header.data_size();
    }
    void block::set_data_size( const size_t& a_size )
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

    void block::set_cleanup( block_cleanup* a_cleanup )
    {
        delete a_cleanup;
        f_cleanup = a_cleanup;
        return;
    }


}
