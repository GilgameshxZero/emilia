# Todo

* Separate logs into separate streams to separate destinations.
* Capture all exceptions in main thread and deal with them there.
* Check scripts/HTTP server to make sure that GET requests deal with URI conversion properly.
* Build a more unified `push` and `pull` protocol.
* Use of passwords should be domain-specific.
* Blocking server/clients should be standardized in `Rain`.
* Separate structs and params into their own files in `Rain`.
* Define a '\r\n' line ending in `Rain`.
* Implement a true message system in `Rain` without `RainWindow`.
* Change all threads in `Rain` to C++11 threads.