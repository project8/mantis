#include "mt_network_writer.hh"

#include <cstring> // for memcpy()
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::stringstream;

namespace mantis
{
    network_writer::network_writer( buffer* a_buffer, condition* a_condition ) :
            writer( a_buffer, a_condition ),
            f_record_dist( new record_dist() )
    {
    }
    network_writer::~network_writer()
    {
        delete f_record_dist;
    }

    void network_writer::initialize( request* a_request )
    {
        writer::initialize( a_request );

        cout << "[network_writer] opening write connection..." << endl;

        client* t_write_client = new client( a_request->write_host(), a_request->write_port() );
        f_record_dist->set_connection( t_write_client );

        return;
    }

    bool network_writer::write( block* a_block )
    {
        return f_record_dist->push_record( a_block );
    }

}
