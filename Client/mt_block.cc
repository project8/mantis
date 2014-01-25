#include "mt_block.hh"

namespace mantis
{

    block_base::block_base() :
            f_header()
    {
        f_header.set_state( block_header_state_t_written );
        f_header.set_acquisition_id( 0 );
        f_header.set_record_id( 0 );
        f_header.set_timestamp( 0 );
        f_header.set_data_size( 0 );
    }

    block_base::~block_base()
    {
    }

    block_header_state_t block_base::get_state() const
    {
        return f_header.state();
    }
    void block_base::set_state( block_header_state_t a_state )
    {
        f_header.set_state( a_state );
        return;
    }

    bool block_base::is_acquiring() const
    {
        if( f_header.state() == block_header_state_t_acquiring )
        {
            return true;
        }
        return false;
    }
    void block_base::set_acquiring()
    {
        f_header.set_state( block_header_state_t_acquiring );
        return;
    }

    bool block_base::is_acquired() const
    {
        if( f_header.state() == block_header_state_t_acquired )
        {
            return true;
        }
        return false;
    }
    void block_base::set_acquired()
    {
        f_header.set_state( block_header_state_t_acquired );
        return;
    }

    bool block_base::is_writing() const
    {
        if( f_header.state() == block_header_state_t_writing )
        {
            return true;
        }
        return false;
    }
    void block_base::set_writing()
    {
        f_header.set_state( block_header_state_t_writing );
        return;
    }

    bool block_base::is_written() const
    {
        if( f_header.state() == block_header_state_t_written )
        {
            return true;
        }
        return false;
    }
    void block_base::set_written()
    {
        f_header.set_state( block_header_state_t_written );
        return;
    }

    acquisition_id_type block_base::get_acquisition_id() const
    {
        return f_header.acquisition_id();
    }
    void block_base::set_acquisition_id( const acquisition_id_type& an_id )
    {
        f_header.set_acquisition_id( an_id );
        return;
    }

    record_id_type block_base::get_record_id() const
    {
        return f_header.record_id();
    }
    void block_base::set_record_id( const record_id_type& an_id )
    {
        f_header.set_record_id( an_id );
        return;
    }

    time_nsec_type block_base::get_timestamp() const
    {
        return f_header.timestamp();
    }
    void block_base::set_timestamp( const time_nsec_type& a_timestamp )
    {
        f_header.set_timestamp( a_timestamp );
        return;
    }

    size_t block_base::get_data_size() const
    {
        return f_header.data_size();
    }
    void block_base::set_data_size( const size_t& a_size )
    {
        f_header.set_data_size( a_size );
        return;
    }

    block_header* block_base::header()
    {
        return &f_header;
    }

    const block_header* block_base::header() const
    {
        return &f_header;
    }


}
