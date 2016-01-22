#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS

#include "mt_file_writer.hh"

#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_acq_request.hh"

#include "logger.hh"
#include "param.hh"
#include "path.hh"

#include "M3Exception.hh"
#include "M3Types.hh"
#include "M3Version.hh"

#include <limits> // for numeric_limits<>::max
#include <cstring> // for memcpy()

using std::stringstream;

// for using numeric_limits<>::max in windows
#define NOMINMAX

using scarab::param_node;
using scarab::param_value;
using scarab::path;

namespace mantis
{

    LOGGER( mtlog, "file_writer" );

    MT_REGISTER_WRITER( file_writer, "file" );

    file_writer::file_writer() :
            writer(),
            f_monarch( NULL ),
            f_header( NULL ),
            f_stream( NULL ),
            f_record( NULL ),
            f_data( NULL ),
            f_last_acquisition_id( std::numeric_limits< uint32_t >::max() ),
            f_dev_mgr( NULL )
    {
    }
    file_writer::~file_writer()
    {
        delete f_monarch;
    }

    void file_writer::set_device_manager( device_manager* a_dev_mgr )
    {
        f_dev_mgr = a_dev_mgr;
        return;
    }

    bool file_writer::initialize_derived( acq_request* a_acq_request )
    {
        INFO( mtlog, "opening file..." );

        const param_value* t_file_config = a_acq_request->value_at( "file" );
        const param_value* t_desc_config = a_acq_request->value_at( "description" );
        const param_node* t_acq_config = a_acq_request->node_at( "acquisition" );
        if( t_file_config == NULL || t_acq_config == NULL )
        {
            ERROR( mtlog, "Either the file configuration (" << t_file_config << ") or mantis config (" << t_acq_config << ") is missing" );
            return false;
        }
        const param_node* t_all_devs_config = t_acq_config->node_at( "devices" );
        if( t_all_devs_config == NULL )
        {
            ERROR( mtlog, "The device configuration is missing" );
            return false;
        }

        try
        {
            path t_filename( scarab::expand_path( t_file_config->as_string() ) );
            try
            {
                f_monarch = monarch3::Monarch3::OpenForWriting( t_filename.native() );
            }
            catch( monarch3::M3Exception& e )
            {
                ERROR( mtlog, "error opening file: " << e.what() );
                return false;
            }
            catch( std::exception& e )
            {
                ERROR( mtlog, "Non-Monarch error opening file: " << e.what() );
                return false;
            }
            f_header = f_monarch->GetHeader();

            // run header information
            f_header->SetFilename( t_filename.native() );
            if( t_desc_config != NULL ) f_header->SetDescription( t_desc_config->as_string() );
            f_header->SetRunDuration( t_acq_config->get_value< double >( "duration" ) );
            char t_timestamp[64];
            scarab::get_time_absolute_str( t_timestamp );
            f_header->SetTimestamp( t_timestamp );

            // stream and channel information
            for( param_node::const_iterator t_node_it = t_all_devs_config->begin(); t_node_it != t_all_devs_config->end(); ++t_node_it )
            {
                std::string t_dev_name( t_node_it->first );
                const param_node* t_device_config;
                try
                {
                    t_device_config = &( t_node_it->second->as_node() );
                }
                catch( exception& e )
                {
                    WARN( mtlog, "Ignoring non-param_node object in \"devices\": <" << t_dev_name << ">" );
                    continue;
                }

                // if these aren't present, an exception will be thrown, which will be caught since this is in a try block
                uint32_t t_data_mode = t_device_config->get_value< uint32_t >( "data-mode" );
                uint32_t t_chan_mode = t_device_config->get_value< uint32_t >( "channel-mode" );
                unsigned t_rate = t_device_config->get_value< unsigned >( "rate" );
                unsigned t_sample_size = t_device_config->get_value< unsigned >( "sample-size" );
                unsigned t_n_channels = t_device_config->get_value< unsigned >( "n-channels" );
                std::vector< unsigned > t_chan_vec;
                if( t_n_channels == 1 )
                {
                    f_header->AddStream( std::string( "mantis - " ) + t_dev_name,
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params(0).data_type_size, t_data_mode,
                        f_dev_mgr->device()->params(0).bit_depth,
                        f_dev_mgr->device()->params(0).bits_right_aligned ? monarch3::sBitsAlignedRight : monarch3::sBitsAlignedLeft,
                        &t_chan_vec );
                }
                else
                {
                    f_header->AddStream( std::string( "mantis - " ) + t_dev_name, t_n_channels, t_chan_mode,
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params(0).data_type_size, t_data_mode,
                        f_dev_mgr->device()->params(0).bit_depth,
                        f_dev_mgr->device()->params(0).bits_right_aligned ? monarch3::sBitsAlignedRight : monarch3::sBitsAlignedLeft,
                        &t_chan_vec );
                }

                unsigned i_chan_mantis = 0; // this is the channel number in mantis, as opposed to the channel number in the monarch file
                for( std::vector< unsigned >::const_iterator it = t_chan_vec.begin(); it != t_chan_vec.end(); ++it )
                {
                    f_header->GetChannelHeaders()[ *it ].SetVoltageOffset( f_dev_mgr->device()->params( i_chan_mantis ).v_offset );
                    f_header->GetChannelHeaders()[ *it ].SetVoltageRange( f_dev_mgr->device()->params( i_chan_mantis ).v_range );
                    f_header->GetChannelHeaders()[ *it ].SetDACGain( f_dev_mgr->device()->params( i_chan_mantis ).dac_gain );
                    ++i_chan_mantis;
                }
            }

        }
        catch( scarab::error& e )
        {
            ERROR( mtlog, "Configuration error: " << e.what() );
            return false;
        }
        catch( exception& e )
        {
            ERROR( mtlog, "Mantis error: " << e.what() );
            return false;
        }
        catch( std::exception& e )
        {
            ERROR( mtlog, "std::exception caught: " << e.what() );
            return false;
        }
        INFO( mtlog, "writing header..." );

        try
        {
            f_monarch->WriteHeader();
        }
        catch( monarch3::M3Exception& e )
        {
            ERROR( mtlog, "error while writing header: " << e.what() );
            return false;
        }

        f_stream = f_monarch->GetStream( 0 );
        f_record = f_stream->GetStreamRecord();
        f_data = f_record->GetData();

        return true;
    }

    bool file_writer::write( block* a_block )
    {
        bool t_is_new_acquisition = a_block->get_acquisition_id() != f_last_acquisition_id;
        f_last_acquisition_id = a_block->get_acquisition_id();
        f_record->SetRecordId( (monarch3::RecordIdType) (a_block->get_record_id()) );
        f_record->SetTime( (monarch3::TimeType) (a_block->get_timestamp()) );
        ::memcpy( f_data, a_block->data_bytes(), a_block->get_data_nbytes() );

        return f_stream->WriteRecord( t_is_new_acquisition );
    }

    void file_writer::finalize_derived( param_node* /*a_response*/ )
    {
        DEBUG( mtlog, "File writer finalizing" );
        f_monarch->FinishWriting();
        delete f_monarch;
        f_monarch = NULL;
        return;
    }

}
