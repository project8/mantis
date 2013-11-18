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

    record_dist::record_dist()
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
        if( ! push_data( a_block->data(), flags ) )
        {
            cerr << "[record_dist] a write error occurred while pushing a block's data" << endl;
            return false;
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
        if( ! pull_data( a_block->data(), flags ) )
        {
            cerr << "[record_dist] a read error occurred while pulling a block's data" << endl;
            return false;
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
        return false;
        /*
        size_t t_block_size = reset_buffer( block_size( a_block ) );
        if( ! serialize_block( a_block ) )
            return false;
        try
        {
            f_connection->send( f_buffer, t_block_size, flags );
        }
        catch( exception& e )
        {
            cerr << "a write error occurred while pushing a record: " << e.what() << endl;
            return false;
        }
        return true;
         */
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
        return false;
        /*
        size_t t_block_size = f_buffer_size;
        try
        {
            t_block_size = f_connection->recv_size( flags );
            if( t_block_size == 0 ) return false;
            reset_buffer( t_block_size );
            if( f_connection->recv( f_buffer, t_block_size, flags ) == 0 )
                return false;
        }
        catch( exception& e )
        {
            cerr << "a read error occurred while pulling a record: " << e.what() << endl;
            return false;
        }
        return deserialize_block( a_block );
         */
    }

}
