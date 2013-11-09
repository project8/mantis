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
### Configuration
Executables are configured in four stages, with each stage able to overwrite 
settings from the previous stages:
1. Default configuration (hard-coded)
2. Configuration file (json format)
3. json given on the command line (NOTE: does not currently work)
4. Individual command-line options

#### Configuration files
The configuration file is specified on the command line with config=[filename]

Examples:
- Server
```
{
    "port": 4587
}
```

- Client
```
{
    "port": 4587,
    "host": "localhost",
    "file": "some-file.egg",
    "rate": "500",
    "duration": "100"
}
``` 
#### Command-line options
You can use individual command-line options to overwrite specific configuration
values.  The format for options is: [name]/[type]=[value]
For example:
- port/u=12345
- file/s="a-different-filename.egg"

The types are bool (b), integer (i), unsigned integer (u), double (d), and string (s).

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
$> mantis_px1500_server config=server-config.json
```

### Client
This program is intended to be run on anyone's computer or the server machine itself.  
When run it submits a request object to the server, after which it receives a series 
of status objects.  After a received status object indicates the run is complete, 
the client program spits out the run summary. Usage is:

```
$> mantis_client config=client-config.json file/s=new-filename.egg description/s="this is an awesome run"
```

### Standalone
There's also a program that you can run on the server machine by itself, almost exactly 
previous versions of Mantis.  Usage is:

```
$> mantis_px1500_standalone config=standalone-config.json
```

Potential Issues
----------------
* Running multiple instances of the server, the standalone, or either in combination will likely crash any acquisitions taking place
* All the programs are still extremely chatty since they're freshly debugged
* Once running, the server port is wide open and therefore totally vulnerable
* Killing the server is done with a harsh ctrl-C which we ought to do more gracefully
* Starting the server has to be done manually; might it fit better to start on boot?
* There's no way to even optionally tell a run from the command line whether it's signal, background or some other type
