/*
 * mantis_px1500_standalone.cc
 *
 *      Author: Dan Furse
 *
 *  Standalone DAQ operation using the px1500 digitizer.
 *
 *  Usage:
 *  $> mantis_px1500_standalone file=<filename> description=<description> mode=<1 or 2> rate=<sampling rate> duration=<sampling duration>
 *
 *  Arguments:
 *  - file        (string; required):  egg filename
 *  - description (string; optional):  describe the run
 *  - mode        (integer; required): '1' for single-channel; '2' for double-channel
 *  - rate        (float; required):   digitization rate in MHz
 *  - duration    (float; required):   length of the run in ms
 *
 */

#include "mt_configurator.hh"
#include "mt_server_config.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "mt_buffer.hh"
#include "mt_digitizer_px1500.hh"
#include "mt_writer.hh"
#include "mt_thread.hh"
#include "thorax.hh"
using namespace mantis;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    standalone_config t_sc;
    configurator t_config( argc, argv, &t_sc );

    request t_request;
    response t_response;

    cout << "[mantis_standalone] making request..." << endl;

    t_request.set_file( t_config.get_string_required( "file" ) );
    t_request.set_description( t_config.get_string_optional( "description", "default standalone run" ) );
    t_request.set_date( get_absolute_time_string() );
    t_request.set_mode( (request_mode_t)t_config.get_uint_required( "mode" ) );
    t_request.set_rate( t_config.get_double_required( "rate" ) );
    t_request.set_duration( t_config.get_double_required( "duration" ) );

    cout << "[mantis_standalone] making condition and buffer..." << endl;

    condition* t_condition = new condition();
    buffer* t_buffer = new buffer( t_config.get_uint_required( "buffer-size" ), t_config.get_uint_required( "record-size" ) );

    cout << "[mantis_standalone] making digitizer..." << endl;

    digitizer* t_digitizer = new digitizer_px1500();
    t_digitizer->allocate( t_buffer, t_condition );

    cout << "[mantis_standalone] making writer..." << endl;

    writer* t_writer = new writer( t_buffer, t_condition );

    cout << "[mantis standalone] initializing digitizer..." << endl;

    t_digitizer->initialize( &t_request );

    cout << "[mantis standalone] initializing writer..." << endl;

    t_writer->initialize( &t_request );

    cout << "[mantis standalone] starting threads..." << endl;

    thread* t_digitizer_thread = new thread( t_digitizer );
    thread* t_writer_thread = new thread( t_writer );

    t_digitizer_thread->start();

    while( t_condition->is_waiting() == false )
    {
        usleep( 1000 );
    }

    t_writer_thread->start();

    t_digitizer_thread->join();
    t_writer_thread->join();

    cout << "[mantis standalone] cleaning threads..." << endl;

    delete t_digitizer_thread;
    delete t_writer_thread;

    cout << "[mantis standalone] finalizing digitizer..." << endl;

    t_digitizer->finalize( &t_response );

    cout << "[mantis standalone] finalizing writer..." << endl;

    t_writer->finalize( &t_response );

    cout << "[mantis standalone] cleaning up..." << endl;

    delete t_digitizer;
    delete t_writer;
    delete t_condition;
    delete t_buffer;

    cout << "[mantis standalone] ...done" << endl;

    return 0;
}
