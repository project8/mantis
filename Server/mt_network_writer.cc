#include "mt_network_writer.hh"

#include <cstring> // for memcpy()
#include <iostream>
#include <sstream>
using std::cout;
using std::cerr;
using std::endl;
using std::stringstream;

namespace mantis
{
    network_writer::network_writer( buffer* a_buffer, condition* a_condition ) :
            writer( a_buffer, a_condition ),
            f_record_dist( NULL ),
            f_client( NULL ),
            f_data_chunk_size( 1024 )
    {
    }
    network_writer::~network_writer()
    {
        delete f_client;
        delete f_record_dist;
    }

    void network_writer::initialize( request* a_request )
    {
        writer::initialize( a_request );

        cout << "[network_writer] opening write connection..." << endl;

        f_record_dist = new record_dist();
        f_record_dist->set_data_chunk_size( f_data_chunk_size );

        f_client = new client( a_request->write_host(), a_request->write_port() );
        f_record_dist->set_connection( f_client );

        return;
    }

    void network_writer::finalize( response* a_response )
    {
        // push empty block to indicate end of run
        block t_block;
        if(! f_record_dist->push_record( &t_block ) )
        {
            cerr << "[network_writer] there was an error pushing the end-of-run block" << endl;
        }

        writer::finalize( a_response );

        delete f_client;
        f_client = NULL;

        delete f_record_dist;
        f_record_dist = NULL;
    }

    bool network_writer::write( block* a_block )
    {
        return f_record_dist->push_record( a_block );
    }

    size_t network_writer::get_data_chunk_size() const
    {
        return f_data_chunk_size;
    }
    void network_writer::set_data_chunk_size( size_t size )
    {
        f_data_chunk_size = size;
        return;
    }


}
