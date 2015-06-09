Usage
=====

The available arguments for mantis_server, mantis_client, and any future executables will be collected here. The config option names a file of configuration values in json format. Values passed from the command line override those in the configuration file.

# mantis_server

* config (string) -- filename/path for json configuration file.
* port (integer) -- the port number to open for communication with clients
* digitizer (string; element of [px1500, px14400, test]) -- name of the digitizer to use
* buffer-size (integer) -- number of records (see record-size below) comprising the DMA buffer
* record-size -- (integer) -- the number of digitizer samples in each record

# mantis_client

* config (string) -- filename/path for json configuration file.
* file-writer (string; element of [server, client]) -- specify which process writes files
* port (integer) -- port number to use to connect to server, must match the port argument of the server
* host (string) -- IP address or domain name for the server
* client-port (integer) -- only used in if file-writer="client"; port over which data are transfered from the server to the client
* client-host (string) -- only used in if file-writer="client"; IP address or domain name of client
* filename (string) -- absolute path and filename of the data file to be created
* rate (integer) -- digitization rate in MHz
* duration (integer) -- time length of data file in ms
* description (string; preferably json) -- a description and comments to be added to the egg file's header
* mode (integer) -- digitizer-specific mode of operation
  * PX1500:
    * 0: single channel
    * 1: dual-channel; interleaved samples
    * 2: dual-channel; separated samples
