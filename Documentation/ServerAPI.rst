==========
Server API
==========

The server communicates with `dripline-standard <https://github.com/project8/hardware/wiki/Wire-Protocol>`_ AMQP messages.

The API documentation below is organized by message type.

The routing key should consist of the server queue name, optionally followed by other dot-delimited specifiers.
For each of the message types, the allowed routing key specifiers will be listed.

OP_RUN
======

The `run` message type is used to start an acquisition.

Routing Key Specifiers
^^^^^^^^^^^^^^^^^^^^^^

None

Payload
^^^^^^^

- ``file=[filename; string]`` -- *(required)* Filename for the acquisition.
- ``description=[description; string]`` -- *(optional)* Text description for the acquisition; saved in the file header.



OP_GET
======

The `get` message is used to request the status of various components of the server.

Routing Key Specifiers
^^^^^^^^^^^^^^^^^^^^^^

The information being requested should be specified.  The options are:

- ``acq-config`` -- Returns the current acquisition configuration.
- ``server-config`` -- Returns the current full configuration for the server.
- ``acq-status`` -- Returns the status of an acquisition request.
- ``server-status`` -- Returns the status of the server, including the queue, server worker (digitizer & writer), and request receiver.
- ``queue`` -- Returns the current acquisition queue, including the UUID and filename for each acquisition request.
- ``queue-size`` -- Returns the size of the acquisition queue.

Payload
^^^^^^^

acq-status
----------
- ``value=[UUID; string]`` -- *(required)* UUID of the run being queried.



OP_SET
======

The `set` message type is used to set a value to a parameter in the acquisition configuration.

Routing Key Specifiers
^^^^^^^^^^^^^^^^^^^^^^

The configuration parameter being set should be specified.

Payload
^^^^^^^

- ``value=[value; varied]`` -- *(required)* Specify the value to which the run-configuration item should be set.  Any values valid in the JSON standard will work, including strings, numbers, and ``true`` or ``false`` for booleans.



OP_CMD
======

The `cmd` message type is used to run a variety of different command instructions.  The instruction can be specified either as a routing key specifier or as the first element in the ``values`` array in the payload.

Command Instructions
^^^^^^^^^^^^^^^^^^^^

- ``add.device`` -- Adds a device to the master run configuration. Requires that the device be specified as an instruction option (see Payload section below).
- ``remove.device.[device name]`` -- Removes a device from the master run configuration.
- ``replace-config`` -- Replaces the server's run configuration with the contents of the instruction options, or the JSON file specified in those options (see below).
- ``cancel-acq`` -- Remove an acquisition that is waiting to run from the queue.
- ``clear-queue`` -- Clear scheduled acquisitions from the queue.
- ``start-queue`` -- Start processing the requests in the queue (if the queue had previously been stopped)
- ``stop-queue`` -- Stop processing the requests in the queue (queue is left intact and acquisition in progress is not stopped; restart with ``start-queue``)
- ``stop-acq`` -- Stop any acquisition that is currently running (queue processing will continue).
- ``stop-all`` -- Stop processing the requests in the queue and any acquisition that is currently running.  
- ``quit-mantis`` -- Stop execution of the Mantis server.


Routing Key Specifiers
^^^^^^^^^^^^^^^^^^^^^^

The command instruction can be specified as a routing key specifier.  See the available instructions above.
If the instruction is not given here, it will be expected as the first component of the ``values`` array in the payload.

Payload
^^^^^^^

If the command instruction is not included in the routing key specifier, it should be given in the payload:

- ``values=[[instruction]]`` -- *(optional)* Command instructions can also be specified as the first element of the ``values`` array.

Other payload options depend on the command instruction:

add.device
----------
- ``[device type]=[device name; string]`` -- *(required)* The device type should be one of the valid device types for the server being run.  The device name is the name that will be used to refer to this particular instance of the device in the server configuration.

run.config
----------
- ``load.json=[filename; string]`` -- *(optional)* This JSON file will be parsed by the client, and the contents (plus any other instruction options given) will be used by the server to replace the run configuration.

