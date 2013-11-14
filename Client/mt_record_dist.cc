#include "mt_record_dist.hh"

#include "mt_exception.hh"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring> // for memcpy

using std::cerr;
using std::cout;
using std::endl;

namespace mantis
{

    record_dist::record_dist() :
                    f_state_offset( 0 ),
                    f_acquisition_id_offset( 0 ),
                    f_record_id_offset( 0 ),
                    f_timestamp_offset( 0 ),
                    f_data_size_offset( 0 ),
                    f_data_offset( 0 )
    {
        // f_state_offset = 0 (already set)
        f_acquisition_id_offset = f_state_offset + sizeof( block::state_type );
        f_record_id_offset = f_acquisition_id_offset + sizeof( acquisition_id_type );
        f_timestamp_offset = f_record_id_offset + sizeof( record_id_type );
        f_data_size_offset = f_timestamp_offset + sizeof( time_nsec_type );
        f_data_offset = f_data_size_offset + sizeof( size_t );
        cout << "[record dist]: offsets: " << f_acquisition_id_offset << "  " << f_record_id_offset << "  " << f_timestamp_offset << "  " << f_data_size_offset << "  " << f_data_offset << endl;
    }
    record_dist::~record_dist()
    {
    }

    block::state_type record_dist::get_state() const
    {
        cout << "[record dist] state: " << (block::state_type)(f_buffer + f_state_offset) << endl;
        return (block::state_type)(f_buffer + f_state_offset);
    }
    acquisition_id_type record_dist::get_acquisition_id() const
    {
        cout << "[record dist] acq id: " << (acquisition_id_type)(f_buffer + f_acquisition_id_offset) << endl;
        return (acquisition_id_type)(f_buffer + f_acquisition_id_offset);
    }
    record_id_type record_dist::get_record_id() const
    {
        cout << "[record dist] rec id: " << (record_id_type)(f_buffer + f_record_id_offset) << endl;
        return (record_id_type)(f_buffer + f_record_id_offset);
    }
    time_nsec_type record_dist::get_timestamp() const
    {
        cout << "[record dist] rec dist: " << (time_nsec_type)(f_buffer + f_timestamp_offset) << endl;
        return (time_nsec_type)(f_buffer + f_timestamp_offset);
    }
    size_t record_dist::get_data_size() const
    {
        cout << "[record dist] data size: " << (size_t)(f_buffer + f_data_size_offset) << endl;
        return (size_t)(f_buffer + f_data_size_offset);
    }
    const data_type* record_dist::get_data() const
    {
        cout << "[record dist] data[0]: " << (data_type*)(f_buffer + f_data_offset)[0] << endl;
        return (data_type*)(f_buffer + f_data_offset);
    }
    bool record_dist::push_record( const block* a_block )
    {
        size_t t_block_size = reset_buffer( block_size( a_block ) );
        if( ! serialize_block( a_block ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_block_size );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a record: " << e.what() << endl;
            return false;
        }
        return true;
    }
    bool record_dist::pull_record( block* a_block )
    {
        size_t t_block_size = f_buffer_size;
        try
        {
            t_block_size = f_connection->recv_size();
            if( t_block_size == 0 ) return false;
            reset_buffer( t_block_size );
            if( f_connection->recv( f_buffer, t_block_size ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a record: " << e.what() << endl;
            return false;
        }
        return deserialize_block( a_block );
    }

    size_t record_dist::block_size( const block* a_block ) const
    {
        return sizeof( block::state_type ) +
                sizeof( acquisition_id_type ) +
                sizeof( record_id_type ) +
                sizeof( time_nsec_type ) +
                sizeof( size_t ) +
                a_block->get_data_size();
    }

    bool record_dist::serialize_block( const block* a_block )
    {
        ( block::state_type )(f_buffer + f_state_offset ) = a_block->get_state();
        ( acquisition_id_type )(f_buffer + f_acquisition_id_offset ) = a_block->get_acquisition_id();
        ( record_id_type )(f_buffer + f_record_id_offset ) = a_block->get_record_id();
        ( time_nsec_type )(f_buffer + f_timestamp_offset ) = a_block->get_timestamp();
        ( size_t )(f_buffer + f_data_size_offset ) = a_block->get_data_size();
        ::memcpy( f_buffer + f_data_offset, a_block->data(), a_block->get_data_size() );
        return true;
    }

    bool record_dist::deserialize_block( block* a_block )
    {
        a_block->set_state( ( block::state_type )(f_buffer + f_state_offset ) );
        a_block->set_acquisition_id( ( acquisition_id_type )(f_buffer + f_acquisition_id_offset ) );
        a_block->set_record_id( ( record_id_type )(f_buffer + f_record_id_offset ) );
        a_block->set_timestamp( ( time_nsec_type )(f_buffer + f_timestamp_offset ) );
        a_block->set_data_size( ( size_t )(f_buffer + f_data_size_offset ) );
        ::memcpy( a_block->data(), f_buffer + f_data_offset, a_block->get_data_size() );
    }
}
