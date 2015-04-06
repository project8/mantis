// set a particular value in master-config.run
// expects a single value in the payload called "value"
mantis_client do=config dest=[queue].devices.pxie1.enable value=1
mantis_client do=config dest=[queue].duration value=100

// replace the master-config.run configuration with the contents of my_config.json
// client will read my_config.json and put the contents in the payload
mantis_client do=config dest=[queue] load.json=my_config.json

// add a device of a particular type, and give it a name
// the payload should contain a single name:value pair with the name being the device type, and the value being the name to be used for the device
mantis_client do=config dest=[queue].add.device pxie5122=my_pxie [other devices]

// remove a device with a particular name
// the payload will not be used
mantis_client do=config dest=[queue].remove.device.my_pxie

// return server configuration information and optionally save it in a file
// dest=config returns master-config.run
// dest=server-config returns master-config
mantis_client do=get dest=[queue].config save.json=my_config.json

// adds a run to the queue
// payload should contain a single node with at least the filename
mantis_client do=run dest=[queue] file.filename=my_file.egg file.desc="some sort of data"




//**************************//

mantis_client [options]

Usage examples

* Read client configuration from a json file
    mantis_client config=my_config.json
    
* Specify the broker address
    mantis_client amqp/broker=myrna.local

* Add a PXIe5122 digitizer called pxie1
    mantis_client request=set add/device/pxie5122=pxie1

* Set the "enable" value of the pxie1 digitizer to 1
    mantis_client request=set set/devices/pxie1/enable=1
    
* Get the master configuration from the server
    mantis_client request=get get=config


Full option list

Load a client config file (added flexibility compared to configuring by command line argument):
  config=[json file]

Specify broker:
  amqp/broker=[location (default=localhost)]
  amqp/broker-port=[port # (default=5672)]
  amqp/route=[routing key (default=mantis)]
  amqp/exchange=[exchange name (default=requests)]
  
Request type:
  request=run
  request=set
  request=get
  
For "run" requests:
  file/filename=[filename (default=mantis_client_out.egg)]
  file/description=[run description (default=???)]
  
For "set" requests:
  set/[config option to modify]=[new config value]
  load/json=[json config file to be used by the server; not to be confused with the "config" option for the client]
  add/device/[digitizer type]=[name]
  remove/device=[name]
  
For "get" requests
  get=config save/json=[filename]
  get=status (this option is not yet available)