#include "MantisEgg.hpp"

//REMOVE
#include <iostream>

MantisEgg::MantisEgg() :
    file_ptr( NULL ),
    header_finished( false ),
    file_name( "default_egg.egg" ),
    data_size( sizeof(MantisBufferRecord::DataType) ),
    data_width( 4194304 )
{ /* no-op */
}

MantisEgg::~MantisEgg()
{
    if( file_ptr )
    {
        fclose( file_ptr );
    }
}

MantisEgg* MantisEgg::egg_from_env( safeEnvPtr& env )
{
    MantisEgg* egg_ptr = new MantisEgg();

    if( egg_ptr )
    {
        egg_ptr->file_ptr = fopen( (env.get())->getFileName().c_str(), "wb" );
        if( egg_ptr->file_ptr )
        {
            // Now encode the data size and features, and write it as a string.
            egg_ptr->add_header_attr( "data_format", "id", sizeof(MantisBufferRecord::IndexType) );
            egg_ptr->add_header_attr( "data_format", "timestamp", sizeof(MantisBufferRecord::TimeStampType) );
            egg_ptr->add_header_attr( "data_format", "data", (env.get())->getRecordLength() * sizeof(MantisBufferRecord::DataType) );

            egg_ptr->add_header_attr( "run", "length [ms]", (env.get())->getRunDuration() );
            egg_ptr->add_header_attr( "digitizer", "rate [mHz]", (env.get())->getAcquisitionRate() );
            egg_ptr->add_header_attr( "digitizer", "channels [#]", (env.get())->getChannelMode() );
        }
        else
        {
            delete egg_ptr;
            egg_ptr = NULL;
        }
    }

    return egg_ptr;
}

std::string MantisEgg::attr_to_xml( egg_hdr_attr_k_t key, egg_hdr_attr_v_t val )
{
    std::stringstream fuse;
    fuse << key << "=\"" << val << "\"";
    return fuse.str();
}

std::string MantisEgg::xml_vers_header()
{
    return "<?xml version=\"1.0\"?>";
}

std::string MantisEgg::xml_hdr_open()
{
    return "<header>";
}

std::string MantisEgg::xml_hdr_close()
{
    return "</header>";
}

bool MantisEgg::write_raw_bytes( const void* tgt, std::size_t tgt_size, std::size_t tgt_width )
{
    bool res = true;

    std::size_t written = fwrite( tgt, tgt_size, tgt_width, this->file_ptr );
    if( written != tgt_size * tgt_width )
    {
        res = false;
    }

    return res;
}

bool MantisEgg::write_header()
{
    bool ret_val = true;
    std::string hdr_string;
    std::stringstream fuse;
    egg_hdr_container::iterator header_it = this->header_attrs.begin();

    if( fseek( this->file_ptr, 0, SEEK_SET ) )
    {
        ret_val = false;
    }

    if( ret_val )
    {
        fuse << MantisEgg::xml_vers_header();
        fuse << MantisEgg::xml_hdr_open();

        // Write header attributes
        while( header_it != header_attrs.end() )
        {
            fuse << "<" << (*header_it).first << " ";
            egg_hdr_attr_list::iterator attr_it = (*header_it).second.begin();
            while( attr_it != (*header_it).second.end() )
            {
                std::string xml = MantisEgg::attr_to_xml( (*attr_it).first, (*attr_it).second );
                fuse << xml << " ";
                attr_it++;
            }
            fuse << "/>";
            header_it++;
        }

        // Close the header, we're done.
        fuse << MantisEgg::xml_hdr_close();
    }

    char buffer[2 * sizeof(std::size_t)];
    sprintf( buffer, "%08x", (unsigned) fuse.str().length() );

    this->write_raw_bytes( buffer, sizeof(unsigned char), strlen( buffer ) );

    this->write_raw_bytes( fuse.str().c_str(), sizeof(unsigned char), fuse.str().length() );

    if( ret_val )
    {
        this->header_finished = true;
    }

    return ret_val;
}

bool MantisEgg::write_data( MantisBufferRecord* block )
{
    /* fmt: |fIndex|fTimeStamp|fData| */
    static serializer< MantisBufferRecord::IndexType > index_byter;
    index_byter.value = block->Index();
    static serializer< MantisBufferRecord::TimeStampType > timestamp_byter;
    timestamp_byter.value = block->TimeStamp();

    this->write_raw_bytes( index_byter.value_bytes, sizeof(index_byter.value_bytes[0]), sizeof(index_byter) );
    this->write_raw_bytes( timestamp_byter.value_bytes, sizeof(timestamp_byter.value_bytes[0]), sizeof(timestamp_byter) );
    this->write_raw_bytes( block->DataPtr(), this->data_size, this->data_width );
    return true;
}
