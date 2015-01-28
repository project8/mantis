#include "mt_file_writer.hh"

#include "mt_configurator.hh"
#include "mt_device_manager.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_run_description.hh"

#include "MonarchException.hpp"
#include "MonarchVersion.hpp"

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
            f_record( NULL ),
            f_dev_mgr( NULL )
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
        MTINFO( mtlog, "Opening file" );

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
            string t_filename( t_file_config->get_value( "filename" ) );
            try
            {
                f_monarch = monarch::Monarch::OpenForWriting( t_filename );
            }
            catch( monarch::MonarchException& e )
            {
                MTERROR( mtlog, "Error opening file: " << e.what() );
                return false;
            }
            f_header = f_monarch->GetHeader();

            f_header->SetDescription( t_file_config->get_value( "description" ) );

            f_header->SetFilename( t_filename );
            unsigned t_n_channels = 1;
            unsigned t_req_mode = t_device_config->get_value< unsigned >( "mode" );
            if( t_req_mode == monarch::sFormatSingle )
            {
                f_header->SetAcquisitionMode( monarch::sOneChannel );
                f_header->SetFormatMode( monarch::sFormatSingle );
                t_n_channels = 1;
            }
            if( t_req_mode == monarch::sFormatMultiSeparate )
            {
                f_header->SetAcquisitionMode( monarch::sTwoChannel );
                f_header->SetFormatMode( monarch::sFormatMultiSeparate );
                t_n_channels = 2;
            }
            if( t_req_mode == monarch::sFormatMultiInterleaved )
            {
                f_header->SetAcquisitionMode( monarch::sTwoChannel );
                f_header->SetFormatMode( monarch::sFormatMultiInterleaved );
                t_n_channels = 2;
            }
            f_header->SetAcquisitionRate( t_device_config->get_value< double >( "rate" ) );
            f_header->SetRunDuration( t_run_config->get_value< double >( "duration" ) );
            f_header->SetRecordSize( f_buffer->block_size() / t_n_channels );

            char t_timestamp[64];
            get_time_absolute_str( t_timestamp );
            f_header->SetTimestamp( t_timestamp );
            f_header->SetRunType( monarch::sRunTypeSignal );
            f_header->SetRunSource( monarch::sSourceMantis );

            f_header->SetDataTypeSize( f_dev_mgr->device()->params().data_type_size );
            f_header->SetBitDepth( f_dev_mgr->device()->params().bit_depth );
            f_header->SetVoltageMin( f_dev_mgr->device()->params().v_min );
            f_header->SetVoltageRange( f_dev_mgr->device()->params().v_range );
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


        MTINFO( mtlog, "Writing header" );

        try
        {
            f_monarch->WriteHeader();
        }
        catch( monarch::MonarchException& e )
        {
            MTERROR( mtlog, "Error while writing header: " << e.what() );
            return false;
        }
        f_monarch->SetInterface( monarch::sInterfaceInterleaved );
        f_record = f_monarch->GetRecordInterleaved();

        return true;
    }

    bool file_writer::write( block* a_block )
    {
        f_record->fAcquisitionId = (monarch::AcquisitionIdType) (a_block->get_acquisition_id());
        f_record->fRecordId = (monarch::RecordIdType) (a_block->get_record_id());
        f_record->fTime = (monarch::TimeType) (a_block->get_timestamp());
        ::memcpy( f_record->fData, a_block->data_bytes(), a_block->get_data_nbytes() );

        return f_monarch->WriteRecord();
    }

}
