#include "mt_writer.hh"

#include "mt_iterator.hh"
#include "mt_logger.hh"
#include "mt_param.hh"

#include <cstring> // for memcpy()
using std::stringstream;

namespace mantis
{
    MTLOGGER( mtlog, "writer" );

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

    bool writer::initialize( run_description* a_run_desc )
    {
        f_canceled = false;

        //MTINFO( mtlog, "resetting counters..." );

        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;

        return initialize_derived( a_run_desc );
    }
    void writer::execute()
    {
        iterator t_it( f_buffer, "writer" );

        timespec t_start_time;
        timespec t_stop_time;

        IT_TIMER_SET_IGNORE_INCR( t_it )
        while( +t_it == true )
            ;
        IT_TIMER_UNSET_IGNORE_INCR( t_it )

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
                    MTINFO( mtlog, "releasing buffer" );
                    f_condition->release();
                }
                ++t_it;
            }

            //if the block we're on is unused, skip it
            if( t_it->is_unused() == true )
            {
                continue;
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
                    MTINFO( mtlog, "was canceled mid-run" );
                    f_cancel_condition.release();
                }
                else
                {
                    MTINFO( mtlog, "finished normally" );
                }
                return;
            }

            //write the block
            t_it->set_writing();

            //MTDEBUG( mtlog, "writer:" );
            //f_buffer->print_states();

            if( write( t_it.object() ) == false )
            {
                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                MTINFO( mtlog, "finished abnormally because writing failed" );
                return;
            }

            if( f_acquisition_count == t_it.object()->get_acquisition_id() )
            {
                f_acquisition_count++;
            }
            f_record_count++;

            t_it->set_written();

            //MTINFO( mtlog, "records written: " << f_record_count );

        }

        return;
    }
    void writer::cancel()
    {
        //cout << "CANCELING WRITER" );
        if( ! f_canceled.load() )
        {
            f_canceled.store( true );
            f_cancel_condition.wait();
        }
        //cout << "  writer has finished canceling" );
        return;
    }
    void writer::finalize( param_node* a_response )
    {
        //MTINFO( mtlog, "calculating statistics..." );
        double t_livetime = (double) (f_live_time) * SEC_PER_NSEC;
        double t_mb_written = (double) (4 * f_record_count);

        param_node* t_resp_node = new param_node();
        param_value t_value;
        t_resp_node->add( "record-count", t_value << f_record_count );
        t_resp_node->add( "acquisition-count", t_value << f_acquisition_count );
        t_resp_node->add( "livetime", t_value << t_livetime );
        t_resp_node->add( "mb-written", t_value << t_mb_written );
        t_resp_node->add( "writer-rate", t_value << t_mb_written / t_livetime );

        a_response->add( "file-writer", t_resp_node );

        finalize_derived( a_response );

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
