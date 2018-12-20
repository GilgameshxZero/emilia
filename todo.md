# Todo

* Separate logs into separate streams to separate destinations.
* Check scripts/HTTP server to make sure that GET requests deal with URI conversion properly.
* Cleanup restart/exit routine.
* `Rain` optimizations.
  * Separate structs and params into their own files in `Rain`.
  * Define a '\r\n' line ending in `Rain`.
  * Implement a true message system in `Rain` without `RainWindow`.
  * Change all threads in `Rain` to C++11 threads.
  * ClientSocketManager should not shut down send thread, just block, so that we don't incur so many penalties for sending/creating new threads.