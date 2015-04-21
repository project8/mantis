#define MANTIS_API_EXPORTS
#define M3_API_EXPORTS

#include "mt_file_writer.hh"

#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_run_description.hh"

#include "M3Exception.hh"
#include "M3Types.hh"
#include "M3Version.hh"

#include <cstring> // for memcpy()
using std::stringstream;

namespace mantis
{
    MTLOGGER( mtlog, "file_writer" );

    MT_REGISTER_WRITER( file_writer, "file" );

    file_writer::file_writer() :
            writer(),
            f_monarch( NULL ),
            f_header( NULL ),
            f_stream( NULL ),
            f_record( NULL ),
            f_data( NULL ),
            f_last_acquisition_id( UINT32_MAX ),
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

    bool file_writer::initialize_derived( run_description* a_run_desc )
    {
        MTINFO( mtlog, "opening file..." );

        const param_value* t_file_config = a_run_desc->value_at( "file" );
        const param_value* t_desc_config = a_run_desc->value_at( "description" );
        const param_node* t_mantis_config = a_run_desc->node_at( "mantis-config" );
        if( t_file_config == NULL || t_mantis_config == NULL )
        {
            MTERROR( mtlog, "Either the file configuration (" << t_file_config << ") or mantis config (" << t_mantis_config << ") is missing" );
            return false;
        }
        const param_node* t_all_devs_config = t_mantis_config->node_at( "devices" );
        if( t_all_devs_config == NULL )
        {
            MTERROR( mtlog, "The device configuration is missing" );
            return false;
        }

        try
        {
            std::string t_filename( t_file_config->as_string() );
            try
            {
                f_monarch = monarch3::Monarch3::OpenForWriting( t_filename );
            }
            catch( monarch3::M3Exception& e )
            {
                MTERROR( mtlog, "error opening file: " << e.what() );
                return false;
            }
            f_header = f_monarch->GetHeader();

            // run header information
            f_header->SetFilename( t_filename );
            if( t_desc_config != NULL ) f_header->SetDescription( t_desc_config->as_string() );
            f_header->SetRunDuration( t_mantis_config->get_value< double >( "duration" ) );
            char t_timestamp[64];
            get_time_absolute_str( t_timestamp );
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
                    MTWARN( mtlog, "Ignoring non-node param object in \"devices\": <" << t_dev_name << ">" );
                    continue;
                }

                uint32_t t_data_mode = t_device_config->get_value< uint32_t >( "data-mode" );
                uint32_t t_chan_mode = t_device_config->get_value< uint32_t >( "channel-mode" );
                unsigned t_n_channels = t_device_config->get_value< unsigned >( "n-channels" );
                unsigned t_rate = t_device_config->get_value< unsigned >( "rate" );
                unsigned t_sample_size = t_device_config->get_value< unsigned >( "sample-size" );
                std::vector< unsigned > t_chan_vec;
                if( t_n_channels == 1 )
                {
                    f_header->AddStream( std::string( "mantis - " ) + t_dev_name,
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params().data_type_size, t_data_mode,
                        f_dev_mgr->device()->params().bit_depth, &t_chan_vec );
                }
                else
                {
                    f_header->AddStream( std::string( "mantis - " ) + t_dev_name, t_n_channels, t_chan_mode,
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params().data_type_size, t_data_mode,
                        f_dev_mgr->device()->params().bit_depth, &t_chan_vec );
                }

                for( std::vector< unsigned >::const_iterator it = t_chan_vec.begin(); it != t_chan_vec.end(); ++it )
                {
                    f_header->GetChannelHeaders()[ *it ].SetVoltageOffset( t_device_config->get_value< double >( "voltage-offset" ) );
                    f_header->GetChannelHeaders()[ *it ].SetVoltageRange( t_device_config->get_value< double >( "voltage-range" ) );
                    f_header->GetChannelHeaders()[ *it ].SetDACGain( t_device_config->get_value< double >( "dac-gain" ) );
                }
            }

        }
        catch( param_exception& e )
        {
            MTERROR( mtlog, "Configuration error: " << e.what() );
            return false;
        }
        catch( exception& e )
        {
            MTERROR( mtlog, "Mantis error: " << e.what() );
            return false;
        }
        catch( std::exception& e )
        {
            MTERROR( mtlog, "std::exception caught: " << e.what() );
            return false;
        }
        MTINFO( mtlog, "writing header..." );

        try
        {
            f_monarch->WriteHeader();
        }
        catch( monarch3::M3Exception& e )
        {
            MTERROR( mtlog, "error while writing header: " << e.what() );
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
        MTDEBUG( mtlog, "File writer finalizing" );
        f_monarch->FinishWriting();
        delete f_monarch;
        f_monarch = NULL;
        return;
    }

}
