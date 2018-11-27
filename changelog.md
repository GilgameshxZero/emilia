# Changelog

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