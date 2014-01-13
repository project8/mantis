Mantis
======
Mantis is the data-acquisition software for the Project 8 collaboration.

The package is divided into a Server component that runs the digitizer and 
optionally records the data files, and a Client component that makes run requests and 
optionally writes the data files.  The client and server can be run on the same machine, 
or communicate with one another over a network.

It is currently able to record data taken from a Signatec PX1500 digitizer.

Dependencies
------------
- Google protocol buffers
- PX1500 driver software (if building the server with the PX1500 enabled)

Monarch, a previous dependency, is now included as a submodule.

Other external packages included in the distributed code include rapidJSON and the
Boost atomic library.

Installing
----------
Mantis is installed using CMake (version 2.8 or better).
We recommend an out-of-source build:
```
  >  mkdir build
  >  cd build
```
    
To configure the installation you can use cmake, ccmake, or cmake-gui.
To build the server executable, enable `Mantis_BUILD_SERVER`.  A test digitizer 
that creates a sawtooth time series is automatically included. 
Other optionally-available digitizers are specified with `Mantis_INCLUDE_[type-of-digitizer]_SERVER`:
- `Mantis_INCLUDE_PX1500_SERVER` for using the PX1500

To build and install:
```
  >  make
  >  make install
```
    
The install prefix is specified by the CMake variable `CMAKE_INSTALL_PREFIX`.
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
 3. ~~json given on the command line~~ (NOTE: does not currently work)
 4. Individual command-line options

#### Available configuration options
- Server
    - `port` (integer; required) -- port number to open for communication with clients
    - `digitizer` (string; required) -- name of the digitizer to use:
        - `px1500` -- Signatec PX1500
    - `buffer-size` (integer; required) -- the number of records that make up the DAQ buffer
    - `record-size` (integer; required) -- the number of samples in each record

- Client
    - `file-writer` (string; required) -- specify whether the client or server writes the files
        - `server` -- data is written by the server; no data is transferred over the network connection
        - `client` -- data is transferred to and written to disk by the client
    - `port` (integer; required) -- port number to use for communication with the server
    - `host` (string; required) -- address at which to contact the server
        - localhost` -- for servers and clients running on the same machine
        - `xxx.xxx.xxx.xxx` -- IP address of the server
        - `some.address.edu` -- Domain name of the server
    - `client-port` (integer; optional) -- port number of the client for file writing; this is only needed 
    if the client will be writing files, and even so, if it is not provided, port+1 will be used
    - `client-host` (string; optional) -- host address of the client for file writing; this is only needed 
    if the client will be writing files.  See "host" for available options.
    - `filename` (string; required) -- path and name of the data file to be created
    - `rate` (integer; required) -- digitization rate in MHz
    - `duration` (integer; required) -- time length of the data file in ms
    - `description` (string; required) -- a description string for the egg header
    - `mode` (integer; required) -- digitizer-specific mode of operation
        - `PX1500`
            -  `0` = single channel
            -  `1` = dual-channel; interleaved samples
            -  `2` = dual-channel; separated samples

All required configuration options have a hard-coded default, so a value will be set even if it isn't specified by the user.

#### Configuration files
The configuration file is specified on the command line with `config=[filename]`

Examples:
- Server
```
    {
        "port": 4587,
        "digitizer": "px1500",
        "buffer-size": 512,
        "record-size": 4194304
    }
```

- Client
```
    {
        "port": 4587,
        "host": "localhost",
        "file": "some-file.egg",
        "rate": "500.0",
        "duration": "1000.0"
    }
``` 
#### Command-line options
You can use individual command-line options to overwrite specific configuration
values.  The format for options is: `[name]/[type]=[value]`
For example:
- `port/i=12345`
- `file/s="a-different-filename.egg"`

The types are bool (`b`), integer (`i`), double (`d`), and string (`s`).

### Executables
Both the client and server can be exited nicely using either `ctrl-c` or `ctrl-\`. 

(Technical details: `SIGINT` and `SIGQUIT` are both caught and will initiate a shutdown of 
the client and server nicely. This signals can be used by other applications 
to control the server or client.)

#### Server
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
$> mantis_server config=server-config.json
```

#### Client
This program is intended to be run on anyone's computer or the server machine itself.  
When run it submits a request object to the server, after which it receives a series 
of status objects.  After a received status object indicates the run is complete, 
the client program spits out the run summary. Usage is:

```
$> mantis_client config=client-config.json file/s=new-filename.egg description/s="this is an awesome run"
```


Potential Issues
----------------
* Once running, the server port is wide open and therefore totally vulnerable
* There's no way to even optionally tell a run from the command line whether it's signal, background or some other type
