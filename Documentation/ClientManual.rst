=============
Client Manual
=============

Purpose
=======

The ``mantis_client`` is a utility that is used to control the ``mantis_server``.  This is done by sending requests using 
`dripline-standard <https://github.com/project8/hardware/wiki/Wire-Protocol>`_ AMQP messages.
The ``mantis_server`` :doc:`API<ServerAPI>` describes the instructions that are available.  This manual describes 
how to use the ``mantis_client`` to access that API.

The full server API is accessible from a general dripline client, such as ``dripline_agent``.  The ``mantis_client`` 
includes some functionality that is specific to Mantis, such as loading the full acquisition configuration from a JSON file.  
That extended functionality will be described below.

Usage
=====

``mantis_client [client options] key=[lockout key] do=[message type] dest=[queue].[mantis target] [server options]``

Examples
^^^^^^^^
The queue name is assumed to be ``mantis`` in these examples.  
The first two examples show the usage of client options, and in all of the later examples, any client options are left out for clarity.

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

The client options allow the user to configure the client itself.  These options do not get passed to the server.

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
The message type tells the server what type of request it's receiving.

The message types used by Mantis are:

:OP_RUN: ``do=run`` -- Queue an acquisition with the current acquisition configuration
:OP_GET: ``do=get`` -- Request information from the server
:OP_SET: ``do=set`` -- Change a setting in the acquisition configuration
:OP_CMD: ``do=cmd`` -- Run a command
  
Target
------
The target is a combination of the AMQP queue name used by the server, and a Mantis-specific specifier that 
tells the server how to direct the request.  In the parlance of dripline and the :doc:`Server API<ServerAPI>`, the 
target is the message routing key, and it combines the queue and the routing key specifier.  

The general form for the target is::

  dest=[queue].[mantis target]
  
The options for the ``mantis target`` are given as Routing Key Specifiers in the :doc:`Server API<ServerAPI>`.

  
Server Options
^^^^^^^^^^^^^^

In general the server options that are passed to the ``mantis_client`` are directly placed into the request payload 
as ``(name: value)`` pairs.  The available options are listed under the Payload sections in the :doc:`Server API<ServerAPI>`.

Special Features
----------------

In addition to those options, the ``mantis_client`` has a few special features that get specified here, but are not 
directly included in the payload.  They depend on the message type and Mantis target:

:any:
  ``save.json=[filename (string)]`` -- *(optional)* File in which to save the information returned.  This is primarily useful for saving the acquisition configuration for loading via the client, or saving the full configuration for loading into the server at startup.

:replace-config:
   ``load.json=[filename]`` -- *(optional)* This JSON file will be parsed by the client, and the contents (plus any other instruction options given) will be used by the server to replace the run configuration.
