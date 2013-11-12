#include "mt_record_dist.hh"

#include "mt_exception.hh"

#include <iostream>
#include <cstdio>
#include <cstdlib>

using std::cerr;
using std::cout;
using std::endl;

#include <google/protobuf/text_format.h>

namespace mantis
{

    record_dist::record_dist() :
                    f_state_offset( 0 ),
                    f_acquisition_id_offset( 0 ),
                    f_record_id_offset( 0 ),
                    f_timestamp_offset( 0 ),
                    f_data_offset( 0 )
    {
        f_acquisition_id_offset = f_state_offset + sizeof( block::state_type );
        f_record_id_offset = f_acquisition_id_offset + sizeof( acquisition_id_type );
        f_timestamp_offset = f_record_id_offset + sizeof( record_id_type );
        f_data_offset = f_timestamp_offset + sizeof( time_nsec_type );
        cout << "[record dist]: offsets: " << f_acquisition_id_offset << "  " << f_record_id_offset << "  " << f_timestamp_offset << "  " << f_data_offset << endl;
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
    const data_type* record_dist::get_data() const
    {
        cout << "[record dist] data[0]: " << (data_type*)(f_buffer + f_data_offset)[0] << endl;
        return (data_type*)(f_buffer + f_data_offset);
    }
    bool record_dist::push_record( const block* a_block )
    {

    }
    bool record_dist::pull_record()
    {

    }

}
