

mantis_client [options]

Usage examples

* Read client configuration from a json file
    mantis_client config=my_config.json
    
* Specify the broker address
    mantis_client amqp/broker=myrna.local

* Add a PXIe5122 digitizer called pxie1
    mantis_client request=set add/device/pxie5122=pxie1

* Set the "enable" value of the pxie1 digitizer to 1
    mantis_client request=set set/device/pxie1/enable=1
    
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
  remove/device/[name]
  
For "get" requests
  get=config save/json=[filename]
  get=status (this option is not yet available)