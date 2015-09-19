=============
Client Manual
=============

Usage
=====

``mantis_client [client options] key=[lockout key] do=[message type] dest=[queue].[mantis target] [instruction options]``

Examples
^^^^^^^^
Queue name is assumed to be ``mantis``.  The first two examples show the usage of client options, and in all of the later examples, any client options are left out for clarity.

* Read client configuration from a JSON file::

    mantis_client config=my_config.json
    
* Specify the broker address::

    mantis_client amqp.broker=myrna.local

* Add a PXIe5122 digitizer called pxie1::

    mantis_client do=cmd dest=mantis.add.device pxie5122=pxie1

* Remove a digitizer called my_px1500::

    mantis_client do=cmd dest=[queue].remove.device.my_px1500

* Set the "enabled" value of the pxie1 digitizer to ``true``::

    mantis_client do=set dest=mantis.devices.pxie1.enabled value=true

* Set the run duration to 100 ms::

    mantis_client do=set dest=mantis.duration value=100
    
* Get the master acquisition configuration from the server; the option to save the configuration as a JSON file is used::

    mantis_client do=get dest=mantis.acq-config save.json=my_config.json

* Replace the server's master acquisition configuration with the contents of the specified file::

    mantis_client do=cmd dest=mantis.acq-config load.json=my_config.json

* Submit an acquisition request to the queue::

    mantis_client do=run dest=mantis file=my_file.egg
    
* Send any request with a lockout key::

    mantis_client key=0123456789abcdef0123456789abcdef [...]


Full option list
================

Client options
^^^^^^^^^^^^^^

Load a client config file. This file is parsed and used by the client.
In it you can specify any option that would otherwise be specified on the command line::

  config=[json file]

Specify the AMQP broker details::

  amqp.broker=[location (default=localhost)]
  amqp.broker-port=[port # (default=5672)]
  amqp.exchange=[exchange name (default=requests)]


Lockout Key
^^^^^^^^^^^

Mantis follows the dripline wire protocol specification for the `lockout feature <https://github.com/project8/hardware/wiki/Wire-Protocol#lockout>`_.

The key can be provided in either supported format::

  key=0123456789abcdef0123456789abcdef
  key=01234567-89ab-cdef-0123-456789abcdef


Message Type and Target
^^^^^^^^^^^^^^^^^^^^^^^

Message Type
------------
Tell Mantis what type of message it's receiving.

The message types used by Mantis are:

:run: ``do=run`` -- Queue an acquisition with the current acquisition configuration
:get: ``do=get`` -- Request information from the server
:set: ``do=set`` -- Change a setting in the acquisition configuration
:cmd: ``do=cmd`` -- Run a command (see below)
  
Target
------
Specify the AMQP queue name and tell Mantis where to direct the instruction.
Typically much of the information about the instruction is encoded in the target.

The general form for the target is::

  dest=[queue].[mantis target]
  
The target is used in different ways for different message types:

:run:
  ``dest=[queue]`` -- No further information is needed for queueing an acquisition request.
    See the instruction options below.

:get:
  ``dest=[queue].acq-config`` -- Returns the current acquisition configuration.

  ``dest=[queue].server-config`` -- Returns the current full configuration for the server.

  ``dest=[queue].acq-status`` -- Returns the status of an acquisition request.

  ``dest=[queue].server-status`` -- Returns the status of the server, including the queue, server worker (digitizer & writer), and request receiver.
  
  ``dest=[queue].queue`` -- Returns the current acquisition queue, including the UUID and filename for each acquisition request.
  
  ``dest=[queue].queue-size`` -- Returns the size of the acquisition queue.

:set:
  ``dest=[queue].[acq config item]`` -- Sets the value of an item in the run configuration.
  Requires the "values" instruction option below.
  Returns the current run configuration.

:cmd:
  ``dest=[queue].add.device`` -- Adds a device to the master run configuration. Requires that the device be specified as an instruction option (see below).

  ``dest=[queue].remove.device.[device name]`` -- Removes a device from the master run configuration.

  ``dest=[queue].replace-config`` -- Replaces the server's run configuration with the contents of the instruction options, or the JSON file specified in those options (see below).
  
  ``dest=[queue].cancel-acq`` -- Remove an acquisition that is waiting to run from the queue.
  
  ``dest=[queue].clear-queue`` -- Clear scheduled acquisitions from the queue.
  
  ``dest=[queue].start-queue`` -- Start processing the requests in the queue (if the queue had previously been stopped)

  ``dest=[queue].stop-queue`` -- Stop processing the requests in the queue (queue is left intact and acquisition in progress is not stopped; restart with ``start-queue``)
  
  ``dest=[queue].stop-acq`` -- Stop any acquisition that is currently running (queue processing will continue).
  
  ``dest=[queue].stop-all`` -- Stop processing the requests in the queue and any acquisition that is currently running.
  
  ``dest=[queue].quit-mantis`` -- Stop execution of the Mantis server.
  
Message Options
^^^^^^^^^^^^^^^

:any:
  ``save.json=[filename]`` -- *(optional)* File in which to save the information returned.  This is primarily useful for saving the run configuration for loading via the client, or saving the full configuration for loading into the server at startup.

:run:
  ``file=[filename]`` -- *(required)* Name of the file that will be created.

  ``description=[description]`` -- *(optional)* Description string
  
:get:
  :acq-status:
    ``value=[value]`` -- *(required)* UUID of the run being queried

:set:
  ``value=[value]`` -- *(required)* Specify the value to which the run-configuration item should be set.  Any values valid in the JSON standard will work, including strings, numbers, and ``true`` or ``false`` for booleans.

:cmd:
  :add.device:
    ``[device type]=[device name]`` -- *(required)* The device type should be one of the valid device types for the server being run.  The device name is the name that will be used to refer to this particular instance of the device in the server configuration.
  :replace-config:
    ``load.json=[filename]`` -- *(optional)* This JSON file will be parsed by the client, and the contents (plus any other instruction options given) will be used by the server to replace the run configuration.

    ``[other run configuration options]`` -- *(optional)* These options (plus any given in a loaded configuration file) will be used by the server to replace the run configuration.
