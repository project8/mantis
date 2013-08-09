#include "parser.hh"
#include "request.pb.h"
#include "response.pb.h"
#include "buffer.hh"
#include "digitizer.hh"
#include "writer.hh"
#include "thread.hh"
#include "time.hh"
using namespace mantis;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    parser t_parser( argc, argv );

    cout << "[mantis_standalone] making run..." << endl;

    context* t_run = new context( NULL );
    request& t_request = t_run->get_request();
    t_request.set_file( t_parser.get_required< string >( "file" ) );
    t_request.set_description( t_parser.get_optional< string >( "description", "testing standalone mantis" ) );
    t_request.set_date( get_string_time() );
    t_request.set_mode( t_parser.get_required< request_mode_t >( "mode") );
    t_request.set_rate( t_parser.get_required< double >( "rate" ) );
    t_request.set_duration( t_parser.get_required< double >( "duration" ) );

    cout << "[mantis_standalone] making condition and buffer..." << endl;

    condition* t_condition = new condition();
    buffer* t_buffer = new buffer( 512 );

    cout << "[mantis_standalone] making digitizer..." << endl;

    digitizer* t_digitizer = new digitizer( t_buffer, t_condition );

    cout << "[mantis_standalone] making writer..." << endl;

    writer* t_writer = new writer( t_buffer, t_condition );

    cout << "[mantis standalone] initializing digitizer..." << endl;

    t_digitizer->initialize( t_run );

    cout << "[mantis standalone] initializing writer..." << endl;

    t_writer->initialize( t_run );

    cout << "[mantis standalone] starting threads..." << endl;

    thread* t_digitizer_thread = new thread( t_digitizer );
    thread* t_writer_thread = new thread( t_writer );

    t_digitizer_thread->start();
    t_writer_thread->start();

    t_digitizer_thread->join();
    t_writer_thread->join();

    cout << "[mantis standalone] cleaning threads..." << endl;

    delete t_digitizer_thread;
    delete t_writer_thread;

    cout << "[mantis standalone] finalizing digitizer..." << endl;

    t_digitizer->finalize( t_run );

    cout << "[mantis standalone] finalizing writer..." << endl;

    t_writer->finalize( t_run );

    cout << "[mantis standalone] cleaning up..." << endl;

    delete t_digitizer;
    delete t_writer;
    delete t_condition;
    delete t_buffer;

    cout << "[mantis standalone] ...done" << endl;

    return 0;
}
