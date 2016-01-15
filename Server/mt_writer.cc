#define MANTIS_API_EXPORTS

#include "mt_writer.hh"

#include "mt_iterator.hh"

#include "logger.hh"
#include "param.hh"

#include <cstring> // for memcpy()
using std::stringstream;

using scarab::param_value;

namespace mantis
{
    LOGGER( mtlog, "writer" );

    using scarab::time_to_nsec;

    writer::writer() :
            f_buffer( NULL ),
            f_condition( NULL ),
            f_canceled( false ),
            f_cancel_condition(),
            f_record_count( 0 ),
            f_acquisition_count( 0 ),
            f_live_time( 0 ),
            f_status( k_ok ),
            f_status_message()
    {
    }
    writer::~writer()
    {
    }

    bool writer::initialize( acq_request* a_run_desc )
    {
        f_canceled.store( false );

        //INFO( mtlog, "resetting counters..." );

        f_record_count = 0;
        f_acquisition_count = 0;
        f_live_time = 0;

        return initialize_derived( a_run_desc );
    }
    void writer::execute()
    {
        if( f_status != k_ok )
        {
            ERROR( mtlog, "Writer status is not \"ok\"" );
            return;
        }

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
                // if attempt fails, see if we're waiting on the buffer condition, and release if so
                if( f_condition->is_waiting() == true )
                {
                    DEBUG( mtlog, "Releasing buffer" );
                    f_condition->release();
                }
                // advance; will wait for mutex lock on the next block
                ++t_it;
            }

            //check if thread has been cancelled
            if (f_canceled.load())
            {
                // to make sure we don't deadlock anything
                if (f_cancel_condition.is_waiting())
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                set_status( k_error, "Finished abnormally because writer thread was canceled" );
                INFO(mtlog, "Finished abnormally because writer thread was canceled");
                return;
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
                    set_status( k_error, "Writer was canceled mid-run" );
                    INFO( mtlog, "Writer was canceled mid-run" );
                    f_cancel_condition.release();
                }
                else
                {
                    set_status( k_ok, "Finished normally" );
                    INFO( mtlog, "Finished normally" );
                }
                return;
            }

            //write the block
            t_it->set_writing();

            //DEBUG( mtlog, "writer:" );
            //f_buffer->print_states();

            if( write( t_it.object() ) == false )
            {
                // to make sure we don't deadlock anything
                if( f_cancel_condition.is_waiting() )
                {
                    f_cancel_condition.release();
                }

                //GET OUT
                set_status( k_error, "Finished abnormally because writing failed" );
                INFO( mtlog, "Finished abnormally because writing failed" );
                return;
            }

            if( f_acquisition_count == t_it.object()->get_acquisition_id() )
            {
                f_acquisition_count++;
            }
            f_record_count++;

            t_it->set_written();

            //INFO( mtlog, "records written: " << f_record_count );

        }

        set_status( k_error, "This section of code should not be reached" );
        return;
    }

    /* Asyncronous cancelation:
    During the main execution loop, the variable f_canceled is checked to see if the thread was canceled.
    Question: what happens if the thread is cancelled when the iterator is waiting to receive a mutex lock to advance to the next block?
    Question: what does the cancel condition do?
    */
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
        //INFO( mtlog, "calculating statistics..." );
        double t_livetime = (double) (f_live_time) * SEC_PER_NSEC;
        double t_mb_written = (double) (4 * f_record_count);

        param_node* t_resp_node = new param_node();
        t_resp_node->add( "record-count", param_value( f_record_count ) );
        t_resp_node->add( "acquisition-count", param_value( f_acquisition_count ) );
        t_resp_node->add( "livetime", param_value( t_livetime ) );
        t_resp_node->add( "mb-written", param_value( t_mb_written ) );
        t_resp_node->add( "writer-rate", param_value( t_mb_written / t_livetime ) );

        a_response->add( "writer", t_resp_node );

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
