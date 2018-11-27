# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>.

See [changelog.md](changelog.md) for information on incremental updates, and [todo.md](todo.md) for information on planned updates.

## Overview

Emilia consists of three components: an HTTP server, a SMTP server, and an update server/client. All three components run under the same process. Commands to the application either access the local servers, or, after connecting to a remote update server, the remote servers. The update server is primarily used for pushing and pulling changes between development and production environments.

## Deployment Overview

There are two types of files: local and remote. 

## Commands

Command | Action
| - | - |
exit | Exits the process, or, if connected to a remote update server, the remote process (which will also disconnect).
help | Lists all available commands.
connect | Connect to remote update server. Not available if already connected to a remote server.
disconnect | Disconnect from a remote connection.
push | Replaces all files on remote server with local files, substituting those in the local 'remote' directory to the remote root directory in the process.
pull | Replaces local files with those on the remote server, ignoring the files in the root with the same paths as those in the local 'remote' directory and replacing those in the 'remote' directory instead.
sync | 
start | Starts the HTTP and SMTP servers.
stop | Stops the HTTP and SMTP servers.
restart | Restarts the HTTP and SMTP servers.