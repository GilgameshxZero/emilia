# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>.

See [changelog.md](changelog.md) for information on incremental updates, and [todo.md](todo.md) for information on planned updates.

## Overview

Emilia consists of three components: an HTTP server, a SMTP server, and an update server/client. All three components run under the same process. Commands to the application either access the local servers, or, after connecting to a remote update server, the remote servers. The update server is primarily used for pushing and pulling changes between development and production environments.

## Deployment Overview

There are two types of files in this project: `shared` and `exclusive`.

* `shared`: The same file should exist between all distributions of this project.
* `exclusive`: A separate version of this file should exist on each distribution of the project.

File types are specified in the config file of a server. Pushing overwrites `shared` files based on the local config. Pulling overwrites files based on the remote config. Sync is only available when file type configurations are the same on local and remote.

## Commands

Command | Action
| - | - |
exit | Exits the process, or, if connected to a remote update server, the remote process (which will also disconnect).
help | Lists all available commands.
connect | Connect to remote update server. Not available if already connected to a remote server.
disconnect | Disconnect from a remote connection.
push | Overwrite the remote `shared` files with the local ones.
push-exclusive | Overwrite the remote `shared` files with the local ones, and the remote `exclusive` files with the local ones as well.
pull | Overwrite the local `shared` files with the remote ones, and create/update a copy of the local `exclusive` files from the remote, but not overwrite the local `exclusive` files.
sync | Update both sides with the most recently modified copies of the `shared` files, and update copies of the `exclusive` files on each end.
start | Starts the HTTP and SMTP servers.
stop | Stops the HTTP and SMTP servers.
restart | Restarts the HTTP and SMTP servers.