#include "block.hh"

#include <sys/time.h>

namespace mantis
{

    block::block() :
            f_state( e_written ),
            f_acquisition_id( 0 ),
            f_record_id( 0 ),
            f_timestamp( 0 ),
            f_data( NULL )
    {
    }

    block::~block()
    {
    }

    bool block::is_acquiring() const
    {
        if( f_state == e_acquiring )
        {
            return true;
        }
        return false;
    }
    void block::set_acquiring()
    {
        f_state = e_acquiring;
        return;
    }

    bool block::is_acquired() const
    {
        if( f_state == e_acquired )
        {
            return true;
        }
        return false;
    }
    void block::set_acquired()
    {
        f_state = e_acquired;
        return;
    }

    bool block::is_writing() const
    {
        if( f_state == e_writing )
        {
            return true;
        }
        return false;
    }
    void block::set_writing()
    {
        f_state = e_writing;
        return;
    }

    bool block::is_written() const
    {
        if( f_state == e_written )
        {
            return true;
        }
        return false;
    }
    void block::set_written()
    {
        f_state = e_written;
        return;
    }

    const acquisition_id_t& block::get_acquisition_id() const
    {
        return f_acquisition_id;
    }
    void block::set_acquisition_id( const acquisition_id_t& an_id )
    {
        f_acquisition_id = an_id;
        return;
    }

    const record_id_t& block::get_record_id() const
    {
        return f_record_id;
    }
    void block::set_record_id( const record_id_t& an_id )
    {
        f_record_id = an_id;
        return;
    }

    const timestamp_t& block::get_timestamp() const
    {
        return f_timestamp;
    }
    void block::set_timestamp( const timestamp_t& a_timestamp )
    {
        f_timestamp = a_timestamp;
        return;
    }

    data_t* block::data()
    {
        return f_data;
    }

    data_t** block::handle()
    {
        return &f_data;
    }

}
