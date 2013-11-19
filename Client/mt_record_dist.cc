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
            f_data_chunk_size( 1024 ),
            f_n_full_chunks( 0 ),
            f_last_data_chunk_size( 0 )
    {
    }
    record_dist::~record_dist()
    {
    }

    bool record_dist::push_record( const block* a_block, int flags )
    {
        if( ! push_header( a_block->header(), flags ) )
        {
            cerr << "[record_dist] a write error occurred while pushing a block's header" << endl;
            return false;
        }

        if( a_block->header()->data_size() > 0 )
        {
            f_n_full_chunks = a_block->header()->data_size() / f_data_chunk_size;
            f_last_data_chunk_size = a_block->header()->data_size() - f_data_chunk_size * f_n_full_chunks;

            if( ! push_data( a_block->data(), flags ) )
            {
                cerr << "[record_dist] a write error occurred while pushing a block's data" << endl;
                return false;
            }
        }

        return true;
    }
    bool record_dist::pull_record( block* a_block, int flags )
    {
        if( ! pull_header( a_block->header(), flags ) )
        {
            cerr << "[record_dist] a read error occurred while pulling a block's header" << endl;
            return false;
        }

        if( a_block->header()->data_size() > 0 )
        {
            f_n_full_chunks = a_block->header()->data_size() / f_data_chunk_size;
            f_last_data_chunk_size = a_block->header()->data_size() - f_data_chunk_size * f_n_full_chunks;

            if( ! pull_data( a_block->data(), flags ) )
            {
                cerr << "[record_dist] a read error occurred while pulling a block's data" << endl;
                return false;
            }
        }
        return true;
    }

    bool record_dist::push_header( const block_header* a_block_header, int flags )
    {
        size_t t_header_size = reset_buffer( a_block_header->ByteSize() );
        cout << "request size to write: " << t_header_size << endl;
        if( ! a_block_header->SerializeToArray( f_buffer, t_header_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_header_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a request: " << e.what() << endl;
            return false;
        }
        return true;
    }

    bool record_dist::push_data( const data_type* a_block_data, int flags )
    {
        const data_type* t_offset_data = a_block_data;
        for( unsigned i_chunk = 0; i_chunk < f_n_full_chunks; ++i_chunk )
        {
            try
            {
                f_connection->send( (char*)( t_offset_data ), f_data_chunk_size, flags );
            }
            catch( exception& e )
            {
                cerr << "a write error occurred while pulling chunk " << i_chunk << " of the data from a record: " << e.what() << endl;
                return false;
            }
            t_offset_data += f_data_chunk_size;
        }

        if( f_last_data_chunk_size > 0 )
        {
            try
            {
                f_connection->send( (char*)( t_offset_data ), f_last_data_chunk_size, flags );
            }
            catch( exception& e )
            {
                cerr << "a write error occurred while pulling the last chunk of the data from a record: " << e.what() << endl;
                return false;
            }
        }

        return true;
    }

    bool record_dist::pull_header( block_header* a_block_header, int flags )
    {
        size_t t_header_size = f_buffer_size;
        try
        {
            t_header_size = f_connection->recv_size( flags );
            if( t_header_size == 0 ) return false;
            reset_buffer( t_header_size );
            if( f_connection->recv( f_buffer, t_header_size, flags ) == 0 )
            {
                cout << "connection read length was 0" << endl;
                return false;
            }
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a request: " << e.what() << endl;
            return false;
        }
        return a_block_header->ParseFromArray( f_buffer, t_header_size );
    }

    bool record_dist::pull_data( data_type* a_block_data, int flags )
    {
        data_type* t_offset_data = a_block_data;
        for( unsigned i_chunk = 0; i_chunk < f_n_full_chunks; ++i_chunk )
        {
            try
            {
                f_connection->recv_size( flags );
                if( f_connection->recv( (char*)( t_offset_data ), f_data_chunk_size, flags ) == 0 )
                    return false;
            }
            catch( exception& e )
            {
                cerr << "a read error occurred while pulling chunk " << i_chunk << " of the data from a record: " << e.what() << endl;
                return false;
            }
            t_offset_data += f_data_chunk_size;
        }

        if( f_last_data_chunk_size > 0 )
        {
            try
            {
                f_connection->recv_size( flags );
                if( f_connection->recv( (char*)( t_offset_data ), f_last_data_chunk_size, flags ) == 0 )
                    return false;
            }
            catch( exception& e )
            {
                cerr << "a read error occurred while pulling the last chunk of the data from a record: " << e.what() << endl;
                return false;
            }
        }

        return true;
    }

    size_t record_dist::get_data_chunk_size()
    {
        return f_data_chunk_size;
    }
    void record_dist::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }
}
