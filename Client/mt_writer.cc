#include "mt_writer.hh"

#include "mt_iterator.hh"

#include <cstring> // for memcpy()
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::stringstream;

namespace mantis
{
    writer::writer() :
            f_buffer( NULL ),
            f_condition( NULL ),
            f_canceled( false ),
            f_cancel_condition(),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 )
    {
    }
    writer::~writer()
    {
    }

    bool writer::initialize( request* a_request )
    {
        f_canceled = false;

        cout << "[writer] resetting counters..." << endl;

        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;

        return initialize_derived( a_request );
    }
    void writer::execute()
    {
        iterator t_it( f_buffer );

        timespec t_start_time;
        timespec t_stop_time;

        while( +t_it == true )
            ;

        //start live timing
        get_time_monotonic( &t_start_time );

        //go go go
        while( true )
        {
            //try to advance
            if( +t_it == false )
            {
                if( f_condition->is_waiting() == true )
                {
                    cout << "[writer] releasing" << endl;
                    f_condition->release();
                }
                ++t_it;
            }

            //if the block we're on is already written, the run is done
            if( t_it->is_written() == true )
            {
                //stop live timing
                get_time_monotonic( &t_stop_time );

                //accumulate live time
                f_live_time = time_to_nsec( t_stop_time ) - time_to_nsec( t_start_time );

                //GET OUT
                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    cout << "[writer] was canceled mid-run" << endl;
                    f_cancel_condition.release();
                }
                else
                {
                    cout << "[writer] finished normally" << endl;
                }
                return;
            }

            //write the block
            t_it->set_writing();
            if( write( t_it.object() ) == false )
            {
                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                cout << "[writer] finished abnormally because writing failed" << endl;
                return;
            }

            if( f_acquisition_count == t_it.object()->get_acquisition_id() )
            {
                f_acquisition_count++;
            }
            f_record_count++;

            t_it->set_written();

            //cout << "[writer] records written: " << f_record_count << endl;

        }

        return;
    }
    void writer::cancel()
    {
        //cout << "CANCELING WRITER" << endl;
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        cout << "  writer has finished canceling" << endl;
        return;
    }
    void writer::finalize( response* a_response )
    {
        cout << "[writer] calculating statistics..." << endl;

        a_response->set_writer_records( f_record_count );
        a_response->set_writer_acquisitions( f_acquisition_count );
        a_response->set_writer_live_time( (double) (f_live_time) * SEC_PER_NSEC );
        a_response->set_writer_megabytes( (double) (4 * f_record_count) );
        a_response->set_writer_rate( a_response->writer_megabytes() / a_response->writer_live_time() );

        return;
    }

    void writer::set_buffer( buffer* a_buffer, condition* a_condition )
    {
        f_buffer = a_buffer;
        f_condition = a_condition;
        return;
    }

    bool writer::get_canceled()
    {
        return f_canceled.load();
    }

    void writer::set_canceled( bool a_flag )
    {
        f_canceled.store( a_flag );
        return;
    }
}
