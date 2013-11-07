/*
 * mantis_px1500_server.cc
 *
 *      Author: Dan Furse
 *
 *  Server component of the DAQ for the px1500 digitizer.
 *
 *  Usage:
 *  $> mantis_px1500_server port=<some port number>
 *
 *  Arguments:
 *  - port (integer; required): port number to be opened by the server
 *
 */

#include "mt_configurator.hh"
#include "mt_server_config.hh"
#include "mt_server.hh"
#include "mt_condition.hh"
#include "mt_queue.hh"
#include "mt_buffer.hh"
#include "mt_receiver.hh"
#include "mt_worker.hh"
#include "mt_digitizer_px1500.hh"
#include "mt_writer.hh"
#include "mt_thread.hh"
using namespace mantis;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    server_config t_sc;
    configurator t_config( argc, argv, &t_sc );

    cout << "[mantis_server] creating objects..." << endl;

    server t_server( t_config.get_string_required( "port" ) );

    condition t_buffer_condition;
    buffer t_buffer( t_config.get_uint_required( "buffer-size" ) );

    digitizer_px1500 t_digitizer( &t_buffer, &t_buffer_condition );
    writer t_writer( &t_buffer, &t_buffer_condition );

    condition t_queue_condition;
    queue t_queue;

    receiver t_receiver( &t_server, &t_queue, &t_queue_condition );
    worker t_worker( &t_digitizer, &t_writer, &t_queue, &t_queue_condition, &t_buffer_condition );

    cout << "[mantis_server] starting threads..." << endl;

    thread t_queue_thread( &t_queue );
    thread t_receiver_thread( &t_receiver );
    thread t_worker_thread( &t_worker );

    t_queue_thread.start();
    t_receiver_thread.start();
    t_worker_thread.start();

    cout << "[mantis_server] joining threads..." << endl;

    t_queue_thread.join();
    t_receiver_thread.join();
    t_worker_thread.join();

    cout << "[mantis_server] shutting down..." << endl;

    return 0;
}

