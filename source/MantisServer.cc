#include "server.hh"
#include "thread.hh"
#include "queue.hh"
#include "receiver.hh"
#include "driver.hh"

using namespace mantis;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::cout;
using std::endl;

int main( int argc, char** argv )
{
    server t_server;
    t_server.open( 51385 );

    queue t_queue;

    receiver t_receiver( &t_server, &t_queue );
    driver t_driver( &t_queue, NULL, NULL );

    thread t_receiver_thread( &t_receiver );
    thread t_driver_thread( &t_driver );

    t_receiver_thread.start();
    t_driver_thread.start();

    t_receiver_thread.join();
    t_driver_thread.join();

    return 0;
}
