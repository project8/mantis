Mantis
======

Mantis is the data-acquisition software package for the Project 8 collaboration.

The main component of Mantis is the server, which runs any number of data-acquisition 
devices.  The server is controlled remotely by a client.  The Mantis package includes 
a client, but other clients can also be used (e.g. [Dripline](https://github.com/project8/dripline)).
The [AMQP](https://www.amqp.org/) protocol is used for communication between the 
client and server.

Mantis can be extended to control a variety of data-acquisition devices.  The devices that 
are or have been supported include:

* National Instruments PXIe5122 (current)
* ROACH1 (incomplete)
* Signatec PX1500 (past)
* Signatec PX14400 (past)
* Keysight U1084A (past; incomplete)

Two software-based test digitizers are also included with Mantis.


Dependencies
------------

**External**
- CMake (3.0 or better)
- Boost (atomic, chrono\*, system\*, uuid; 1.46 or better)
- HDF5\*
- pthread\**
- rabbitmq-c\*
- Drivers for any devices that will be used

\* dependency via a submodule
\*\* not required in Windows

**Submodules** (included with Mantis; must be fetched via Git)
- [Monarch](https://github.com/project8/monarch) (and its submodule, [libthorax](https://github.com/project8/libthorax))
- [SimpleAmqpClient](https://github.com/project8/SimpleAmqpClient)
- [Scarab](https://github.com/project8/scarab)

**Distributed Code** (included with Mantis directly)
- RapidJSON
- msgpack-c


Operating System Support
------------------------

* Mac OS X (usually tested on OS X 10.10)
* Linux (usually tested on Debian Wheezy)
* Windows (usually tested on Windows 7)


Installing
----------

Mantis is installed using CMake (version 3.0 or better).
We recommend an out-of-source build:
```
  >  mkdir build
  >  cd build
```
    
To configure the installation you can use cmake, ccmake, or cmake-gui.
To build the server executable, enable `Mantis_BUILD_SERVER`.  Two test 
digitizers are automatically included.

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

The server is typically started with:
```
  > mantis_server amqp.broker=my.server
```

The formula for running the client is:
```
  > mantis_client [client options] do=[verb] dest=[queue].[mantis destination] [instruction options]
```

For more detailed instructions on running the client, please see the [Client Manual](http://www.project8.org/mantis/ClientManual.html)


Documentation
-------------

Hosted at: http://www.project8.org/mantis


Development
-----------

The Git workflow used is git-flow:
* http://nvie.com/posts/a-successful-git-branching-model/
We suggest that you use the aptly-named git extension, git-flow, which is available from commonly-used package managers:
* https://github.com/nvie/gitflow
