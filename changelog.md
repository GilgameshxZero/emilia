# Changelog

## 6.0.8

* Bugfixes
  * Fixed a concurrency bug which caused the SMTP external client's `ClientSocketManager` to be destructed before its `recvThread` had finished running, raising an exception in `onDisconnect`.
* Quality-of-life
  * Concurrent logs now report the socket number (unique) rather than the IP (non-unique).

## 6.0.7

* Organization
  * Standardize `.gitignore`.

## 6.0.6

* Quality-of-life
  * Fixed some struct initialization warnings.
  * Changed command argument parsing to use lambdas and be more error-tolerant.
  * Total connections are now displayed at the start of console logs.
  * Error reporting has a different format, and prints error code to console.

## 6.0.5

* Features
  * Recognizes multiple SMTP domains.

## 6.0.4

* Quality-of-life
  * Updated to VS2019 toolset.
* Bugfixes
  * Deploy server will no longer terminate connection when new client connects.
  * `Server-Version` header is now served correctly.
  * `ConditionVariable` includes `functional` for `std::bind`.
  * Fixed concurrency issue in `NetworkClientManager` w.r.t. `MessageDoneEvent`, which used to cause crashes after sending emails primarily.

## 6.0.3

* Organization
  * Replaced some `+`s with `,`s in `tsCout`s.
* Quality-of-life
  * New connects on the deploy server will boot the previous.
  * Debug builds now run without DLLs linked.
* Bugfixes
  * `sync` used to fail because it was trying to convert "fail" into a `bool`, which would be `true`. Now it doesn't.

## 6.0.2

* Features
  * `CTRL-C` signals now terminate program as expected.
  * Exiting the program is now accomplished with `WriteConsoleInput`, which allows peaceful termination.
* Organization
  * Removed unnecessary `.gitignore` lines.
  * `bin` no longer in VS project.
  * `build-utils` and `project-utils` combined into `utils` package.
  * Separated out server parameter structs into `http-server-param` and `smtp-server-param`.
  * Command handler functions are no longer capitalized.
  * Moved some magic numbers and strings to `constants`.
  * Restart codes are now defined as an `enum` in `constants`.
  * `HandleRequest` functions merged into `onMessage`.
* Quality-of-life
  * Capitalized build event outputs.
  * `build-inc` now outputs build number.
  * `server` command now outputs remote server status when connected.
* Style
  * Command line interpretation is now a `switch` statement.
  * `include`s are more compact, and ordering consistent.
  * Added docstring to `checksumFileCRC32`.
  * Added error handling for WinAPI functions in `main`.

## 6.0.1

* Features
  * Added environment variable `EMILIA_VERSION` for CGI scripts to retrieve server version.
* QOL
  * Transfer sizes now in MB.
  * `getTime` now allows to get specified `time_t` in format.
  * `ClientSocketManager` now has more comprehensive error reporting.
  * `reportError` now puts error code on same line as error description.
* Bugfixes
  * Removed `static` qualifier on some function variables in `network-base`.
  * Fixed bug cutting off the first character in the first environment variable passed to CGI scripts.
  * `getVersionStr` now removes the final '\0' character.
  * 404 requests now resolve correctly.
  * Crash restarts now start servers automatically correctly.

## 6.0.0

* Conceptual
  * Revamped deployment routine.
  * Revamped configuration specification.
  * Separated logging into separate streams for each server.
  * Added blocking for send as an option for all `SocketManager`s.
* Formatting & code
  * Reformatted some files to have `else` on same line as bracket.
  * Moved command line arguments to `constants.h` file.
  * Batch executable update script and default configuration have been moved to strings within project.
* Deployment
  * Sync now concurrently uploads and downloads.
* Configuration
  * By default, HTTP server has a content-type of `application/octet-stream`, which is not specified in the configuration anymore.
  * Connecting to a remote server can choose which port to connect on as well.
* Bugfixes
  * Fixed multithreading in `NetworkClientManager` causing sporadic failures during sending.

## 5.7.8

* Removed scripts from version control.

## 5.7.7

* Preliminary work on the `Timer` class, which allows for timed waits and timer adjustment.

## 5.7.6

* Update server and client now disconnect if server detects an existing connection with another client.
* Added options in `ClientSocketManager` to enable reconnect on disconnect functionality rather than by default.
* Create a separate `.cpp` for `RecvHandlerParam`.

## 5.7.5

* Add configuration specification section in readme.
* Specify more granular update files.
* Update `.gitignore` and remove some resource files from VC.

## 5.7.4

* Further project build path updates.

## 5.7.3

* Updated project properties to compile to x86.
* Renamed radio script and re-coded it, but it still doesn't work for mobile.

## 5.7.2

* Release build.
* Fix some warnings related to `int` vs `size_t` comparisons.

## 5.7.1

* Added preliminary script to manage Prototype radio.
* Changed build options for more speed optimizations.

## 5.7.0

* Added preliminary support for `range` headers in the HTTP server.
* Headers in the specification can be empty; the server will fill them in automatically.

## 5.6.6

* Added content-type header specs and range header.
* HTTP server now properly terminates threads even when threads are still transferring content on disconnect.
* Moved some HTTP server config parsing to main.
* Optimized some routines in HTTP server response parsing.

## 5.6.5

* Bugfix: Logging in the HTTP server now logs requests as intended.
* Added new script `prototype-stream-random`.

## 5.6.4

* Prints version string on startup.

## 5.6.3

* HTTP server now creates an extra thread for each connection to process messages.
  * This thread is able to be terminated by `recvThread` for fast connection closes.
  * `wait-10-secs` works as expected now.

## 5.6.2

* Revert to 5.5.0 commit because of instability in 5.6.1.
  * Reverts multithreaded HTTP server change.
* Re-add `wait-10-secs` script.

## 5.6.1

* Changed some lambda captures to pointers for speed.

## 5.6.0

* HTTP server now creates new threads to process requests.
* On disconnect, HTTP server terminates all threads associated with a socket.
* Added `wait-10-secs` script to test new functionality.

## 5.5.0

* Block console input until commands are complete.
* Implemented `sync` command.
* Implemented `restart-all` command.
* Created `ConditionVariable` in `Rain`, which safeguards against spurious wakeups.
* Server will boot update connection if not authenticated within 5 seconds of connecting.
* Last access time checking and settings is done using Unix functions, which are much faster.

## 5.4.4

* Change to SMTP connect timeout in config to 5 seconds.

## 5.4.3

* Update server now only pulls files if they are newer than existing.

## 5.4.2

* Added script to modify mailing list subscribers.

## 5.4.1

* Added option to ignore files within the domain-specific exclusive files.

## 5.4.0

* Updated readme to reflect commands.
* Implemented `pull`.
* Bugfix: `Rain` now deals with symbolic links as directories correctly.

## 5.3.4

* HTTP now gives 404 when directory is requested.
* Ignored more files in config.

## 5.3.3

* Move data encoding away from scripts to front-end.

## 5.3.2

* Query string URI conversion is now delayed from HTTP server to scripts, fixing any conversion issues before.
* HTTP server `headerDelim` is fixed.

## 5.3.1

* Fixed crash on `release` runs from `LogStream` after replacing all threads with `std::thread`.
* CSM now creates one send thread in its lifetime instead of many.

## 5.3.0

* Now uses `std::this_thread::sleep_for` instead of `Sleep`.
* Use `std::cout.flush()` instead of `fflush(stdout)`.
* Slightly more informative logging distinguishes between HTTP & SMTP client connects/disconnects and removes `Info:` tag.
* Uses `Rain` integrated line endings instead of `\r\n` which is more platform-independent.
* Replace all uses of `CreateThread` with `std::thread`.

## 5.2.9

* Exit process on restart is more standardized (but still uses `exit`).

## 5.2.8

* Command stream is paused during some commands.
* Add more directories to ignore file during `push`.

## 5.2.7

* Small console output change to server side of `push`.
* Only one instance of the application can run at the same time from the same .exe.
* `push-exclusive` now restarts application with update script when there are un-writable locked files.
* Sending 0 bytes no longer displays as `nan-ind%`.

## 5.2.6

* `push` and `push-exclusive` parameters are stored in delegate parameters instead of as statics of helper functions, making the commands more resistant to failure.

## 5.2.5

* Authentication now uses exclusive configuration-specific passwords instead of general configuration password is `yes` selected.

## 5.2.4

* Failed authentication no longer crashes application.

## 5.2.3

* Console output now fills hexes with 0s and others with spaces.

## 5.2.2

* During `push` commands, file diffing now uses last modified time instead of CRC32 hashes, which is faster.

## 5.2.1

* Finished integrating `headed` servers and clients with the update process.
* Built in `headed` process with `Rain`.
* Standardized some network-related naming in `Rain`.
* Standardized line endings in markdown files.
* Better console formatting during `push` commands.

## 5.2.0

* Added to-do's.
* Cleaned `Rain` headers.
* Started ideating and implementing `headed` socket messages, which have a predefined length like `blocked` messages but are more concise.

## 5.1.7

* Added script to track IPs.
* Changed all scripts to output in HTML content-type.

## 5.1.6

* Created HTTP accessible script to forward requests to internal SMTP server, so that it is accessible through JS.
* Created simple POST and GET scripts to print back requests.
* Refined build pipelines for scripts and main program.
* Coded `getQueryToMap` in `network-utility` which transforms a GET query to a string:string map.
* Opened `server@emilia-tan.com` to the outside world without authentication.
* CGI script specification can now include paths.
* Bugfix: SMTP server not longer ignores requests if they are sent in the same block.

## 5.1.5

* Flush `stdout` more often with `push` on server end.
* Server closes connection on authentication fail.
* Internal SMTP server is now gone; all mail is directly forwarded/sent.

## 5.1.4

* Fixed SMTP server config authentication error.
* Renamed SMTP files to represent client better.

## 5.1.3

* Merged common code between `push` and `push-exclusive`.
* Requires authentication for high-level commands.
* Automatically re-authenticates on reconnect.
* Client shows requested file list on `push`, and `push-exclusive`.
* HTTP & SMTP servers automatically start on startup.

## 5.1.2

* New build versioning makes compiles faster.
* Moved some parameters to external header files.

## 5.1.1

* Implemented `push-exclusive`, and fixed most crashes with `push`.
* Fixed a missing newline log in `push` on the server.
* Moved \exclusive to the ignore set.
* Updated `rmDirRec` to have a optional `want` parameter.
* Updated behavior when no files are requested.
* Update script is now copied to `bin` on build.
* Bugfix: `NetworkClientManager` would crash if `messageDoneEvent` was accessed concurrently, which has now been solved with mutexes.

## 5.1.0

* Implemented `push` command which works most of the time between remote and local.
  * Wrote `update-script.bat` which wait until a file is writable then replaces it.
* Moved the build-ID helper function to a separate file, but this does not speed up compile.
* Implemented `isFileWritable` to test just that.
* Updated main to not crash if update server cannot be run.
* Bugfix: `redirectCerrFile` no longer deletes stream buffer and causes error on use of `cerr`.
* Bugfix: `NetworkClientManager` would sometimes crash when its message queue was modified concurrently.

## 5.0.5

* Updated `getFilesRec` and `getDirsRec` to be able to specify files to want as well as files to ignore.
* Implemented Rain with a CRC32 algorithm.
* Implemented part of the `push` command handshake, with the actual data transfer to go.

## 5.0.4

* Implemented remote `start`, `stop`, and `restart` commands.
* Updated authentication process to disconnect fully on fail.
* Updated `CommandHandlerParam` to its own file.
* Cleaned up logging around connections.
* Bugfix: ServerManager's `listenThread` had race conditions when restarted multiple times in fast succession; fixed by moving mutex locks around.

## 5.0.3

* Implemented local `start`, `stop`, `restart` commands.
* Implemented `connect` and `disconnect` commands.
* Implemented authentication process with update servers.
* Bugfix: ServerManager `disconnectSocket` now works properly.

## 5.0.2

* Designed and summarized commands in readme.

## 5.0.1

* Error reporting with less lines.
* New program icon with transparent background.
* Release application will now restart automatically on crash.
* Bugfix: `static` in some utility-string functions caused problems when multithreaded.

## 5.0.0

* Combined all three servers/clients into one process.
