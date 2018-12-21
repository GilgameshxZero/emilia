# Todo

* Separate logs into separate streams to separate destinations.
* Check scripts/HTTP server to make sure that GET requests deal with URI conversion properly.
* LogStream destructor causes access violation?
* `Rain` optimizations.
  * ClientSocketManager should not shut down send thread, just block, so that we don't incur so many penalties for sending/creating new threads.