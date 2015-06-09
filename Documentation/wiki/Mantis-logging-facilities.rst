Right now there is a singleton called mantis_logger that has 3 methods - Info, Warn, and Error.  The only thing that it really does is timestamp a string that gets passed as an argument to one of these three methods, and then print the result in a nicely formatted way.

It does, however, have a future.  Soon I imagine making this far more robust, extensible, and of course prettier, by turning Info, Warn, and Error into classes in their own right.  These classes will support the operator() to preserve the current interface.  This will also, however, enable streaming of messages into the class by way of something along the lines of
mantis_logger::Info << "here is some information" << std::endl;

wouldn't that be nice?  stay tuned.