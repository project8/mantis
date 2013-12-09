#include "mt_file_writer.hh"

#include "mt_exception.hh"
#include "mt_factory.hh"

#include "MonarchException.hpp"

#include <cstring> // for memcpy()
#include <iostream>
#include <sstream>
using std::cerr;
using std::cout;
using std::endl;
using std::stringstream;

namespace mantis
{
    static registrar< writer, file_writer > s_file_writer_registrar( "file" );

    file_writer::file_writer() :
            writer(),
            f_monarch( NULL ),
            f_header( NULL ),
            f_record( NULL )
    {
    }
    file_writer::~file_writer()
    {
    }

    void file_writer::configure( configurator* )
    {
        return;
    }

    bool file_writer::initialize_derived( request* a_request )
    {
        cout << "[file_writer] opening file..." << endl;

        try
        {
            f_monarch = Monarch::OpenForWriting( a_request->file() );
        }
        catch( MonarchException& e )
        {
            cerr << "[file_writer] error opening file: " << e.what() << endl;
            return false;
        }
        f_header = f_monarch->GetHeader();

        //required fields
        f_header->SetFilename( a_request->file() );
        if( a_request->mode() == request_mode_t_single )
        {
            f_header->SetAcquisitionMode( sOneChannel );
            f_header->SetFormatMode( sFormatSingle );
        }
        if( a_request->mode() == request_mode_t_dual_separate )
        {
            f_header->SetAcquisitionMode( sTwoChannel );
            f_header->SetFormatMode( sFormatMultiSeparate );
        }
        if( a_request->mode() == request_mode_t_dual_interleaved )
        {
            f_header->SetAcquisitionMode( sTwoChannel );
            f_header->SetFormatMode( sFormatMultiInterleaved );
        }
        f_header->SetAcquisitionRate( a_request->rate() );
        f_header->SetRunDuration( a_request->duration() );
        f_header->SetRecordSize( f_buffer->record_size() );

        //optional fields
        f_header->SetTimestamp( a_request->date() );
        f_header->SetDescription( a_request->description() );
        f_header->SetRunType( sRunTypeSignal );
        f_header->SetRunSource( sSourceMantis );
        f_header->SetFormatMode( sFormatSingle );

        cout << "[file_writer] writing header..." << endl;

        f_monarch->WriteHeader();
        f_monarch->SetInterface( sInterfaceInterleaved );
        f_record = f_monarch->GetRecordInterleaved();

        return true;
    }
    bool file_writer::write( block* a_block )
    {
        f_record->fAcquisitionId = (AcquisitionIdType) (a_block->get_acquisition_id());
        f_record->fRecordId = (RecordIdType) (a_block->get_record_id());
        f_record->fTime = (TimeType) (a_block->get_timestamp());
        ::memcpy( f_record->fData, a_block->data(), a_block->get_data_size() );

        return f_monarch->WriteRecord();
    }

}
