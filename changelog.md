# Changelog

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