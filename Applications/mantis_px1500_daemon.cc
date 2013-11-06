/*
 * mantis_px1500_daemon.cc
 *
 *      Author: Dan Furse, Noah Oblath
 *
 *  Daemonized server component of the DAQ for the px1500 digitizer.
 *
 *  Usage:
 *  $> mantis_px1500_daemon port=<some port number>
 *
 *  Arguments:
 *  - port (integer; required): port number to be opened by the server
 *
 */

#include "mt_parser.hh"
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <iostream>


int main( int argc, char** argv )
{
    /* Our process ID and Session ID */
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if( pid < 0 )
    {
        std::cerr << "Fork failed" << std::endl;
        exit( EXIT_FAILURE );
    }
    /* If we got a good PID, then
               we can exit the parent process. */
    if( pid > 0 )
    {
        std::cout << "pid of the child process is " << pid << std::endl;
        exit( EXIT_SUCCESS );
    }

    /* Change the file mode mask */
    umask( 0 );

    /* Open log */
    openlog( "mt_log", LOG_PID | LOG_CONS, LOG_DAEMON );

    /* Create a new SID for the child process */
    sid = setsid();
    if( sid < 0 )
    {
        syslog( LOG_ERR, "[mantis_daemon] unable to create an SID for the child process\n");
        closelog();
        exit( EXIT_FAILURE );
    }

    /* Change the current working directory */
    if( ( chdir("/") ) < 0 )
    {
        syslog( LOG_ERR, "[mantis_daemon] unable to change the working directory\n");
        closelog();
        exit( EXIT_FAILURE );
    }

    /* Close out the standard file descriptors */
    close( STDIN_FILENO );
    close( STDOUT_FILENO );
    close( STDERR_FILENO );

    /* Server initialization */

    parser t_parser( argc, argv );

    syslog( LOG_INFO, "[mantis_daemon] creating objects\n");

    server t_server( t_parser.get_required< int >( "port" ) );

    condition t_buffer_condition;
    buffer t_buffer( 512 );

    digitizer_px1500 t_digitizer( &t_buffer, &t_buffer_condition );
    writer t_writer( &t_buffer, &t_buffer_condition );

    condition t_queue_condition;
    queue t_queue;

    receiver t_receiver( &t_server, &t_queue, &t_queue_condition );
    worker t_worker( &t_digitizer, &t_writer, &t_queue, &t_queue_condition, &t_buffer_condition );

    syslog( LOG_INFO, "[mantis_daemon] starting threads\n");

    thread t_queue_thread( &t_queue );
    thread t_receiver_thread( &t_receiver );
    thread t_worker_thread( &t_worker );

    t_queue_thread.start();
    t_receiver_thread.start();
    t_worker_thread.start();

    syslog( LOG_INFO, "[mantis_daemon] running\n");

    t_queue_thread.join();
    t_receiver_thread.join();
    t_worker_thread.join();

    syslog( LOG_INFO, "[mantis_daemon] shutting down\n");

    exit(EXIT_SUCCESS);
}

