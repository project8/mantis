#include "mt_record_dist.hh"

#include "mt_exception.hh"
#include "mt_logger.hh"

#include <cstdio>
#include <cstdlib>
#include <cstring> // for memcpy

namespace mantis
{
    MTLOGGER( mtlog, "record_dist" );

    record_dist::record_dist() :
            f_data_chunk_nbytes( 1024 ),
            f_n_full_chunks( 0 ),
            f_last_data_chunk_nbytes( 0 )
    {
    }
    record_dist::~record_dist()
    {
    }

    bool record_dist::push_record( const block_base* a_block, int flags )
    {
        if( ! push_header( a_block->header(), flags ) )
        {
            MTERROR( mtlog, "unable to push the block header" );
            return false;
        }

        if( a_block->header()->data_size() > 0 )
        {
            f_n_full_chunks = a_block->get_data_nbytes() / f_data_chunk_nbytes;
            f_last_data_chunk_nbytes = a_block->get_data_nbytes() - f_data_chunk_nbytes * f_n_full_chunks;

            if( ! push_data( a_block->data_bytes(), flags ) )
            {
                MTERROR( mtlog, "unable to push the block data" );
                return false;
            }
        }

        return true;
    }

    bool record_dist::pull_record( block_base* a_block, int flags )
    {
        if( ! pull_header( a_block->header(), flags ) )
        {
            MTERROR( mtlog, "unable to pull the block header" );
            return false;
        }

        // data_size == 0 is allowed, but pull_data would return false, so don't even try if that's what's expected
        if( a_block->header()->data_size() > 0 )
        {
            f_n_full_chunks = a_block->get_data_nbytes() / f_data_chunk_nbytes;
            f_last_data_chunk_nbytes = a_block->get_data_nbytes() - f_data_chunk_nbytes * f_n_full_chunks;

            if( ! pull_data( a_block->data_bytes(), flags ) )
            {
                MTERROR( mtlog, "unable to pull the block data" );
                return false;
            }
        }
        return true;
    }

    bool record_dist::push_header( const block_header* a_block_header, int flags )
    {
        size_t t_header_size = reset_buffer_out( a_block_header->ByteSize() );
        if( ! a_block_header->SerializeToArray( f_buffer_out, t_header_size ) )
            return false;
        try
        {
            f_connection->send( f_buffer_out, t_header_size, flags );
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "closed connection caught by: " << cc.what() );
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "an error occurred while pushing a block header: " << e.what() );
            return false;
        }
        return true;
    }

    bool record_dist::push_data( const char* a_block_data, int flags )
    {
        const char* t_offset_data = a_block_data;
        unsigned i_chunk = 0;
        try
        {
            for( ; i_chunk < f_n_full_chunks; ++i_chunk )
            {
                f_connection->send( t_offset_data, f_data_chunk_nbytes, flags );
                t_offset_data += f_data_chunk_nbytes;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "the connection was closed while pushing chunk " << i_chunk << " of the data from a record;\n"
                    << "detected in <" << cc.what() << ">" );
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "an error occurred while pushing chunk " << i_chunk << " of the data from a record: " << e.what() );
            return false;
        }

        if( f_last_data_chunk_nbytes > 0 )
        {
            try
            {
                f_connection->send( t_offset_data, f_last_data_chunk_nbytes, flags );
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "the connection was closed while pushing the last chunk of the data from a record;\n"
                        << "detected in <" << cc.what() << ">" );
                return false;
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "an error occurred while pushing the last chunk of the data from a record: " << e.what() );
                return false;
            }
        }

        return true;
    }

    bool record_dist::pull_header( block_header* a_block_header, int flags )
    {
        size_t t_header_size = f_buffer_in_size;
        try
        {
            t_header_size = f_connection->recv_type< size_t >( flags );
            if( t_header_size == 0 )
            {
                MTERROR( mtlog, "block_header size was 0" );
                return false;
            }
            reset_buffer_in( t_header_size );
            ssize_t recv_ret = f_connection->recv( f_buffer_in, t_header_size, flags );
            if( recv_ret == (ssize_t)t_header_size )
            {
                return a_block_header->ParseFromArray( f_buffer_in, t_header_size );
            }
            MTERROR( mtlog, "(block_header) received size: " << recv_ret << "; expected size: " << t_header_size );
            return false;
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "connection closed (block_header); detected in <" << cc.what() << ">" );
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "an error occurred while pulling a block header: " << e.what() );
            return false;
        }
        // should not reach here
        return false;
    }

    size_t record_dist::get_data_chunk_nbytes()
    {
        return f_data_chunk_nbytes;
    }
    void record_dist::set_data_chunk_nbytes( size_t nbytes )
    {
        f_data_chunk_nbytes = nbytes;
        return;
    }

    bool record_dist::pull_data( char* a_block_data, int flags )
    {
        char* t_offset_data = a_block_data;
        unsigned i_chunk = 0;
        try
        {
            for( ; i_chunk < f_n_full_chunks; ++i_chunk )
            {
                f_connection->recv_type< size_t >( flags );
                if( f_connection->recv( t_offset_data, f_data_chunk_nbytes, flags ) == 0 )
                    return false;
                t_offset_data += f_data_chunk_nbytes;
            }
        }
        catch( closed_connection& cc )
        {
            MTINFO( mtlog, "the connection was closed while pulling chunk " << i_chunk << " of the data from a record;\n"
                    << "detected in <" << cc.what() << ">" );
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "an error occurred while pulling chunk " << i_chunk << " of the data from a record: " << e.what() );
            return false;
        }

        if( f_last_data_chunk_nbytes > 0 )
        {
            try
            {
                f_connection->recv_type< size_t >( flags );
                if( f_connection->recv( t_offset_data, f_last_data_chunk_nbytes, flags ) == 0 )
                    return false;
            }
            catch( closed_connection& cc )
            {
                MTINFO( mtlog, "the connection was closed while pulling the last chunk of the data from a record;\n"
                        << "detected in <" << cc.what() << ">" );
                return false;
            }
            catch( exception& e )
            {
                MTERROR( mtlog, "an error occurred while pulling the last chunk of the data from a record: " << e.what() );
                return false;
            }
        }

        return true;
    }

}
