#include "writer.hh"

#include "iterator.hh"
#include "time.hh"

#include <cstring> // for memcpy()
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::stringstream;

namespace mantis
{
    writer::writer( buffer* a_buffer, condition* a_condition ) :
            f_buffer( a_buffer ),
            f_condition( a_condition ),
            f_monarch( NULL ),
            f_header( NULL ),
            f_record( NULL ),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 )
    {
    }
    writer::~writer()
    {
    }

    void writer::initialize( run* a_run )
    {
        request& t_request = a_run->get_request();

        cout << "[writer] resetting counters..." << endl;

        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;

        cout << "[writer] opening file..." << endl;

        f_monarch = Monarch::OpenForWriting( t_request.file() );
        f_header = f_monarch->GetHeader();

        //required fields
        f_header->SetFilename( t_request.file() );
        f_header->SetAcquisitionMode( sOneChannel );
        f_header->SetAcquisitionRate( t_request.rate( ) );
        f_header->SetRunDuration( t_request.duration() );
        f_header->SetRecordSize( 4194304 );

        //optional fields
        f_header->SetTimestamp( t_request.date() );
        f_header->SetDescription( t_request.description() );
        f_header->SetRunType( sRunTypeSignal );
        f_header->SetRunSource( sSourceMantis );
        f_header->SetFormatMode( sFormatSingle );

        cout << "[writer] writing header..." << endl;

        f_monarch->WriteHeader();
        f_monarch->SetInterface( sInterfaceInterleaved );
        f_record = f_monarch->GetRecordInterleaved();

        return;
    }

    void writer::execute()
    {
        iterator t_it( f_buffer );

        timestamp_t t_start_time;
        timestamp_t t_stop_time;

        while( +t_it == true )
            ;

        //start live timing
        t_start_time = get_integral_time();

        //go go go
        while( true )
        {
            //try to advance
            if( +t_it == false )
            {
                cout << "[writer] blocked at <" << t_it.index() << ">" << endl;
                if( f_condition->is_waiting() == true )
                {
                    f_condition->release();
                }
                ++t_it;
            }

            //if the block we're on is already written, the run is done
            if( t_it->is_written() == true )
            {
                //stop live timing
                t_stop_time = get_integral_time();

                //accumulate live time
                f_live_time = t_stop_time - t_start_time;

                //GET OUT
                cout << "[writer] finished normally" << endl;
                return;
            }

            //write the block
            t_it->set_writing();
            if( write( t_it.object() ) == false )
            {
                //GET OUT
                cout << "[writer] finished abnormally because writing failed" << endl;
                return;
            }
            t_it->set_written();

        }

        return;
    }

    void writer::finalize( run* a_run )
    {
        response& t_response = a_run->get_response();

        t_response.set_writer_records( f_record_count );
        t_response.set_writer_acquisitions( f_acquisition_count );
        t_response.set_writer_live_time( double( f_live_time ) / double( 1000000 ) );
        t_response.set_writer_megabytes( (double) (4 * f_record_count) );
        t_response.set_writer_rate( (double) (4000000 * f_record_count) / (double) (f_live_time) );

        cout << "[writer] summary:\n";
        cout << "  record count: " << t_response.writer_records() << "\n";
        cout << "  acquisition count: " << t_response.writer_acquisitions() << "\n";
        cout << "  live time: " << t_response.writer_live_time() << "\n";
        cout << "  megabytes: " << t_response.writer_megabytes() << "\n";
        cout << "  rate: " << t_response.writer_rate() << "\n";

        return;
    }

    bool writer::write( block* a_block )
    {
        f_record->fAcquisitionId = (AcquisitionIdType)( a_block->get_acquisition_id() );
        f_record->fRecordId = (RecordIdType)( a_block->get_record_id() );
        f_record->fTime = (TimeType)( a_block->get_timestamp() );
        memcpy( f_record->fData, a_block->data(), 4194304 );

        if( f_monarch->WriteRecord() == false )
        {
            return false;
        }

        if( f_acquisition_count != a_block->get_acquisition_id() )
        {
            f_acquisition_count++;
        }
        f_record_count++;

        return true;
    }

}
