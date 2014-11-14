#include "mt_file_writer.hh"

#include "mt_configurator.hh"
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
            f_dig_params(),
            f_run_desc( NULL )
    {
        // give some reasonable digitizer parameter defaults (these are from the px1500)
        get_calib_params( 8, 1, -0.25, 0.5, &f_dig_params );
    }
    file_writer::~file_writer()
    {
        delete f_run_desc;
    }

    void file_writer::configure( const param_node* a_config )
    {
        // this should be present whether the configuration is from the client or from the server
        get_calib_params(
                a_config->get_value< unsigned >( "bit-depth",      f_dig_params.bit_depth ),
                a_config->get_value< unsigned >( "data-type-size", f_dig_params.data_type_size ),
                a_config->get_value< double   >( "voltage-min",    f_dig_params.v_min ),
                a_config->get_value< double   >( "voltage-range",  f_dig_params.v_range ),
                &f_dig_params );
        return;
    }

    void file_writer::set_run_description( run_description* a_run_desc )
    {
        delete f_run_desc;
        f_run_desc = a_run_desc;
        return;
    }

    bool file_writer::initialize_derived( request* a_request )
    {
        MTINFO( mtlog, "opening file..." );

        try
        {
            f_monarch = monarch::Monarch::OpenForWriting( a_request->file() );
        }
        catch( monarch::MonarchException& e )
        {
            MTERROR( mtlog, "error opening file: " << e.what() );
            return false;
        }
        f_header = f_monarch->GetHeader();

        //required fields
        f_header->SetFilename( a_request->file() );
        unsigned t_n_channels = 1;
        if( a_request->mode() == request_mode_t_single )
        {
            f_header->SetAcquisitionMode( monarch::sOneChannel );
            f_header->SetFormatMode( monarch::sFormatSingle );
            t_n_channels = 1;
        }
        if( a_request->mode() == request_mode_t_dual_separate )
        {
            f_header->SetAcquisitionMode( monarch::sTwoChannel );
            f_header->SetFormatMode( monarch::sFormatMultiSeparate );
            t_n_channels = 2;
        }
        if( a_request->mode() == request_mode_t_dual_interleaved )
        {
            f_header->SetAcquisitionMode( monarch::sTwoChannel );
            f_header->SetFormatMode( monarch::sFormatMultiInterleaved );
            t_n_channels = 2;
        }
        f_header->SetAcquisitionRate( a_request->rate() );
        f_header->SetRunDuration( a_request->duration() );
        f_header->SetRecordSize( f_buffer->record_size() / t_n_channels );

        //optional fields
        f_header->SetTimestamp( a_request->date() );
        f_header->SetRunType( monarch::sRunTypeSignal );
        f_header->SetRunSource( monarch::sSourceMantis );
        f_header->SetDataTypeSize( f_dig_params.data_type_size );
        f_header->SetBitDepth( f_dig_params.bit_depth );
        f_header->SetVoltageMin( f_dig_params.v_min );
        f_header->SetVoltageRange( f_dig_params.v_range );

        // description
        if( f_run_desc == NULL )
        {
            f_run_desc = new run_description();
        }
        f_run_desc->set_mantis_client_exe( a_request->client_exe() );
        f_run_desc->set_mantis_client_version( a_request->client_version() );
        f_run_desc->set_mantis_client_commit( a_request->client_commit() );
        f_run_desc->set_monarch_version( TOSTRING(Monarch_VERSION) );
        f_run_desc->set_monarch_commit( TOSTRING(Monarch_GIT_COMMIT) );
        f_run_desc->set_description( a_request->description() );
        param_node* t_client_config = param_input_json::read_string( a_request->client_config() );
        f_run_desc->set_client_config( *t_client_config );

        string t_full_desc;
        param_output_json::write_string( *f_run_desc, t_full_desc, param_output_json::k_compact );
        f_header->SetDescription( t_full_desc );


        MTINFO( mtlog, "writing header..." );

        try
        {
            f_monarch->WriteHeader();
        }
        catch( monarch::MonarchException& e )
        {
            MTERROR( mtlog, "error while writing header: " << e.what() );
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