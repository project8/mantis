#include "mt_file_writer.hh"

#include "mt_configurator.hh"
#include "mt_exception.hh"
#include "mt_factory.hh"
#include "mt_logger.hh"
#include "mt_run_description.hh"

#include "M3Exception.hh"
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
            f_dig_params(),
            f_run_desc( NULL )
    {
        // give some reasonable digitizer parameter defaults (these are from the px1500)
        get_calib_params( 8, 1, -0.25, 0.5, &f_dig_params );
    }
    file_writer::~file_writer()
    {
        delete f_monarch;
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
            f_monarch = monarch3::Monarch3::OpenForWriting( a_request->file() );
        }
        catch( monarch3::M3Exception& e )
        {
            MTERROR( mtlog, "error opening file: " << e.what() );
            return false;
        }
        f_header = f_monarch->GetHeader();

        // run header information
        f_header->SetFilename( a_request->file() );
        f_header->SetRunDuration( a_request->duration() );
        f_header->SetTimestamp( a_request->date() );

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

        std::string t_full_desc;
        param_output_json::write_string( *f_run_desc, t_full_desc, param_output_json::k_compact );
        f_header->SetDescription( t_full_desc );

        // stream and channel information
        unsigned t_n_channels = 1;
        if( a_request->mode() == request_mode_t_single )
        {
            f_header->AddStream( "mantis digitizer",
                    a_request->rate(), f_buffer->block_size() / t_n_channels,
                    f_dig_params.data_type_size, monarch3::sDigitized,
                    f_dig_params.bit_depth );
            t_n_channels = 1;
        }
        if( a_request->mode() == request_mode_t_dual_separate )
        {
            f_header->AddStream( "mantis digitizer", 2, monarch3::sSeparate,
                    a_request->rate(), f_buffer->block_size() / t_n_channels,
                    f_dig_params.data_type_size, monarch3::sDigitized,
                    f_dig_params.bit_depth );
            t_n_channels = 2;
        }
        if( a_request->mode() == request_mode_t_dual_interleaved )
        {
            f_header->AddStream( "mantis digitizer", 2, monarch3::sInterleaved,
                    a_request->rate(), f_buffer->block_size() / t_n_channels,
                    f_dig_params.data_type_size, monarch3::sDigitized,
                    f_dig_params.bit_depth );
            t_n_channels = 2;
        }

        // write voltage information to channel headers
        typedef std::vector< monarch3::M3ChannelHeader > ChanHeaders;
        for( ChanHeaders::iterator t_chan_it = f_header->GetChannelHeaders().begin(); t_chan_it != f_header->GetChannelHeaders().end(); ++t_chan_it )
        {
            t_chan_it->SetVoltageMin( f_dig_params.v_min );
            t_chan_it->SetVoltageRange( f_dig_params.v_range );
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
