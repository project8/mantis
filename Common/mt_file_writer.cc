#include "mt_file_writer.hh"

#include "mt_configurator.hh"
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
            f_dev_mgr( NULL ),
            f_last_acquisition_id( UINT32_MAX )
    {
    }
    file_writer::~file_writer()
    {
    }

    void file_writer::set_device_manager( device_manager* a_dev_mgr )
    {
        f_dev_mgr = a_dev_mgr;
        return;
    }

    bool file_writer::initialize_derived( run_description* a_run_desc )
    {
        MTINFO( mtlog, "opening file..." );

        const param_node* t_file_config = a_run_desc->node_at( "file" );
        const param_node* t_mantis_config = a_run_desc->node_at( "mantis-config" );
        if( t_file_config == NULL || t_mantis_config == NULL )
        {
            MTERROR( mtlog, "Either the file configuration (" << t_file_config << ") or mantis config (" << t_mantis_config << ") is missing" );
            return false;
        }
        const param_node* t_device_config = t_mantis_config->node_at( "device" );
        const param_node* t_run_config = t_mantis_config->node_at( "run" );
        if( t_device_config == NULL || t_run_config == NULL )
        {
            MTERROR( mtlog, "Either the device configuration (" << t_device_config << ") or run config (" << t_run_config << ") is missing" );
            return false;
        }

        try
        {
            std::string t_filename( t_file_config->get_value( "filename" ) );
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
            f_header->SetRunDuration( t_run_config->get_value< double >( "duration" ) );
            char t_timestamp[64];
            get_time_absolute_str( t_timestamp );
            f_header->SetTimestamp( t_timestamp );

            // stream and channel information
            monarch3::DataFormatType t_data_mode = t_device_config->get_value< monarch3::DataFormatType >( "data-mode" );
            monarch3::MultiChannelFormatType t_chan_mode = t_device_config->get_value< monarch3::MultiChannelFormatType >( "channel-mode" );
            unsigned t_n_channels = t_device_config->get_value< unsigned >( "n-channels" );
            unsigned t_rate = t_device_config->get_value< unsigned >( "rate" );
            unsigned t_sample_size = t_device_config->get_value< unsigned >( "sample-size" );
            if( t_n_channels == 1 )
            {
                f_header->AddStream( "mantis digitizer",
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params().data_type_size, t_data_mode,
                        f_dev_mgr->device()->params().bit_depth );
                t_n_channels = 1;
            }
            else
            {
                f_header->AddStream( "mantis digitizer", t_n_channels, t_chan_mode,
                        t_rate, f_buffer->block_size() / t_n_channels, t_sample_size,
                        f_dev_mgr->device()->params().data_type_size, t_data_mode,
                        f_dev_mgr->device()->params().bit_depth );
                t_n_channels = 2;
            }

            // write voltage information to channel headers
            //TODO: when the device manager is improved, setting of the channel header info will have to go with the streams above
            typedef std::vector< monarch3::M3ChannelHeader > ChanHeaders;
            for( ChanHeaders::iterator t_chan_it = f_header->GetChannelHeaders().begin(); t_chan_it != f_header->GetChannelHeaders().end(); ++t_chan_it )
            {
                t_chan_it->SetVoltageMin( f_dev_mgr->device()->params().v_min );
                t_chan_it->SetVoltageRange( f_dev_mgr->device()->params().v_range );
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

}
