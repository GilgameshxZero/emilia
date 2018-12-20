# Todo

* Separate logs into separate streams to separate destinations.
* Capture all exceptions in main thread and deal with them there.
* Check scripts/HTTP server to make sure that GET requests deal with URI conversion properly.
* Use of passwords should be domain-specific.
* Separate structs and params into their own files in `Rain`.
* Define a '\r\n' line ending in `Rain`.
* Implement a true message system in `Rain` without `RainWindow`.
* Change all threads in `Rain` to C++11 threads.
* Failed authentication crashes the application.
* Console output in fixed-width hex should fill empty spaces with 0.