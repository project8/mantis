Mantis is the data-acquisition software for the Project 8 collaboration.

The package is divided into a Server component that communicates and reads data 
from a digitizer, and a Client component that writes the data to disk.  
The client and server can be run on the same machine, or communicate with one 
another over a network.

It is currently able to record data taken from a Signatec PX1500 digitizer.

Dependencies
------------
- PX1500 driver software (for running in server or standalone modes; unnecessary 
for client-only builds)

Monarch, a previous dependency, is now included as a submodule.

Installing
----------
Mantis is installed using CMake (version 2.8 or better).
We recommend an out-of-source build:
    mkdir build
    cd build
    
To configure the installation you can use cmake, ccmake, or cmake-gui.
To build the server and standalone exectuables, enable Mantis_BUILD_SERVER.
The available digitizers are specified with Mantis_INCLUDE_[type-of-digitizer]_SERVER:
- Mantis_INCLUDE_PX1500_SERVER for using the PX1500

To build and install:
    make
    make install
    
The install prefix is specified by the CMake variable CMAKE_INSTALL_PREFIX.
The library, binaries, and header files will be installed in the 
lib, bin, and include subdirectories. The default install prefix is the
build directory.

Instructions for Use
--------------------
### Server
This program is intended to be run in the background on the server machine.  
It just sits and waits for client programs to submit run request objects.  
When received, the run requests are put onto the back of a queue from whose 
front a worker thread pops the requests, generating the corresponding data files. 
In principle a nearly arbitrary number of stacked requests can be dealt with 
(up to 10 have been tested).  The server keeps track of all submitted requests 
along with their various states of completion and sends out status objects 
every second to waiting clients.  At the end of a run, a summary response 
object with live-time and dead time statistics is sent to the client.
Usage is:

```
$> mantis_px1500_server port=<some port number>
```

### Client
This program is intended to be run on anyone's computer or the server machine itself.  
When run it submits a request object to the server, after which it receives a series 
of status objects.  After a received status object indicates the run is complete, 
the client program spits out the run summary. Usage is:

```
$> mantis_client host=<some host name> port=<some port number> file=<some file name> description=<describe your run> mode=<one or two channel> rate=<sampling rate> duration=<sampling duration>
```

### Standalone
There's also a program that you can run on the server machine by itself, almost exactly 
previous versions of Mantis.  Usage is:

```
$> mantis_px1500_standalone file=<filename> description=<description> mode=<1 or 2> rate=<sampling rate> duration=<sampling duration>
```

Potential Issues
----------------
* Running multiple instances of the server, the standalone, or either in combination will likely crash any acquisitions taking place
* All the programs are still extremely chatty since they're freshly debugged
* Once running, the server port is wide open and therefore totally vulnerable
* Killing the server is done with a harsh ctrl-C which we ought to do more gracefully
* Starting the server has to be done manually; might it fit better to start on boot?
* Port numbers have to be given in manually every time, and if they don't match up, exceptions are thrown; should we pick a port for this and stick with it?
* The description strings are required to be given on the command line; should we pick some kind of default?
* There's no way to even optionally tell a run from the command line whether it's signal, background or some other type
