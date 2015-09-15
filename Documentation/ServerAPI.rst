==========
Server API
==========

The server communicates with `dripline-standard <https://github.com/project8/hardware/wiki/Wire-Protocol>`_ AMQP messages.

The API documentation below is organized by message type.

The routing key should consist of the server queue name, optionally followed by other dot-delimited specifiers.
For each of the message types below, the allowed routing key specifiers will be listed.


Lockout
=======

The server can be locked out to require a key for some types of messages.  Lockable messages are specified below.

The lockout system follows the following rules:

- When a ``lock`` request is made, the UUID key will be returned.  That key should accompany all lockable requests until the server is unlocked.
- When needed, the lock key should be provided in the message payload with the form ``lockout-key=[key; string]``.
- If the lock is enabled, and the key is not provided or is incorrect, then an error will be returned.
- When a ``unlock`` request is made, the return will indicate whether the lock has been successfully disabled.
- If a lockout key is provided but the server is unlocked (or the message is not lockable), it will be ignored.
- In the event that the lock key is lost, the ``unlock`` command can be forced, in which case the server lockout will be disabled without needing the key.


Message Types
=============

OP_RUN
^^^^^^

The `run` message type is used to start an acquisition.

All `run` requests are lockable.

Routing Key Specifiers
----------------------

None

Payload
-------

- ``file=[filename; string]`` -- *(required)* Filename for the acquisition.
- ``description=[description; string]`` -- *(optional)* Text description for the acquisition; saved in the file header.
- ``lockout-key=[key; string]`` -- *(required if locked)* UUID lockout key.



OP_GET
^^^^^^

The `get` message is used to request the status of various components of the server.

No `get` requests are lockable.

Routing Key Specifiers
----------------------

The information being requested should be specified.  The options are:

- ``acq-config`` -- Returns the current acquisition configuration.
- ``server-config`` -- Returns the current full configuration for the server.
- ``acq-status`` -- Returns the status of an acquisition request.
- ``server-status`` -- Returns the status of the server, including the queue, server worker (digitizer & writer), and request receiver.
- ``queue`` -- Returns the current acquisition queue, including the UUID and filename for each acquisition request.
- ``queue-size`` -- Returns the size of the acquisition queue.
- ``is-locked-out`` -- Returns whether or not the server is locked out.

Payload
-------

The payload options depend on the routing key specifier used:

acq-status
""""""""""
- ``value=[UUID; string]`` -- *(required)* UUID of the run being queried.



OP_SET
^^^^^^

The `set` message type is used to set a value to a parameter in the acquisition configuration.

All `set` requests are lockable.

Routing Key Specifiers
----------------------

The configuration parameter being set should be specified.

Payload
-------

- ``value=[value; varied]`` -- *(required)* Specify the value to which the run-configuration item should be set.  Any values valid in the JSON standard will work, including strings, numbers, and ``true`` or ``false`` for booleans.
- ``lockout-key=[key; string]`` -- *(required if locked)* UUID lockout key.



OP_CMD
^^^^^^

The `cmd` message type is used to run a variety of different command instructions.  The instruction can be specified either as a routing key specifier or as the first element in the ``values`` array in the payload.

All `command` requests are lockable.

Command Instructions
--------------------

- ``add.device`` -- Adds a device to the master run configuration. Requires that the device be specified as an instruction option (see Payload section below).
- ``remove.device.[device name]`` -- Removes a device from the master run configuration.
- ``replace-config`` -- Replaces the server's run configuration with the contents of the instruction options, or the JSON file specified in those options (see below).
- ``lock`` -- Requests that the server lockout be enabled. Nothing is done if already locked.
- ``unlock`` -- Requests that the server lockout be disabled.
- ``cancel-acq`` -- Remove an acquisition that is waiting to run from the queue.
- ``clear-queue`` -- Clear scheduled acquisitions from the queue.
- ``start-queue`` -- Start processing the requests in the queue (if the queue had previously been stopped)
- ``stop-queue`` -- Stop processing the requests in the queue (queue is left intact and acquisition in progress is not stopped; restart with ``start-queue``)
- ``stop-acq`` -- Stop any acquisition that is currently running (queue processing will continue).
- ``stop-all`` -- Stop processing the requests in the queue and any acquisition that is currently running.  
- ``quit-mantis`` -- Stop execution of the Mantis server.


Routing Key Specifiers
----------------------

The command instruction can be specified as a routing key specifier.  See the available instructions above.
If the instruction is not given here, it will be expected as the first component of the ``values`` array in the payload.

Payload
-------

- ``values=[[instruction]]`` -- *(optional)* If the command instruction is not included in the routing key specifier, it should be given in the payload as the first element of the ``values`` array.
- ``lockout-key=[key; string]`` -- *(required if locked)* UUID lockout key.

Other payload options depend on the command instruction:

add.device
""""""""""
- ``[device type]=[device name; string]`` -- *(required)* The device type should be one of the valid device types for the server being run.  The device name is the name that will be used to refer to this particular instance of the device in the server configuration.

replace.config
""""""""""""""
- ``load.json=[filename; string]`` -- *(optional)* This JSON file will be parsed by the client, and the contents (plus any other instruction options given) will be used by the server to replace the run configuration.

unlock
""""""
- ``force=[true; bool]`` -- *(optional)* Disables the lockout without a key.

