# Changelog

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
* HTTP server headerDelim is fixed.

## 5.3.1

* Fixed crash on `release` runs from `LogStream` after replacing all threads with `std::thread`.
* CSM now creates one send thread in its lifetime instead of many.

## 5.3.0

* Now uses `std::this_thread::sleep_for` instead of `Sleep`.
* Use `std::cout.flush()` instead of `fflush(stdout)`.
* Slightly more informative logging distinguishes between HTTP & SMTP client connects/disconnects and removes `Info:` tag.
* Uses `Rain` integrated line endings instead of `\r\n` which is more platform-independant.
* Replace all uses of `CreateThread` with `std::thread`.

## 5.2.9

* Exit process on restart is more standardized (but still uses `exit`).

## 5.2.8

* Command stream is paused during some commands.
* Add more directories to ignore file during `push`.

## 5.2.7

* Small console output change to server side of `push`.
* Only one instance of the application can run at the same time from the same .exe.
* `push-exclusive` now restarts application with update script when there are unwritable locked files.
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

* Added todos.
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

* Flush stdout more often with `push` on server end.
* Server closes connection on authentication fail.
* Internal SMTP server is now gone; all mail is directly forwarded/sent.

## 5.1.4

* Fixed SMTP server config authentication error.
* Renamed SMTP files to represent client better.

## 5.1.3

* Merged common code between `push` and `push-exclusive`.
* Requires authentication for high-level commands.
* Automatically re-authenticates on reconnect.
* Client shows requested filelist on `push`, and `push-exclusive`.
* HTTP & SMTP servers automatically start on startup.

## 5.1.2

* New build versioning makes compiles faster.
* Moved some params to external header files.

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