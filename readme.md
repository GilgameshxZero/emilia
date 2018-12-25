# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>. This repository does not contain the front-end files, but does contain the back-end scripts beyond the servers.

See [changelog.md](changelog.md) for information on incremental updates, and [todo.md](todo.md) for information on planned updates.

## Overview

Emilia consists of three components: an HTTP server, a SMTP server, and an update server/client. All three components run under the same process. The update server is primarily used for pushing and pulling changes between development and production environments.

## Deployment Overview

There are two types of files in this project: `shared` and `exclusive`.

* `shared`: The same file should exist between all distributions of this project.
* `exclusive`: A separate version of this file should exist on each distribution of the project.

File types are specified in the config file of a server. `shared` files are those not marked `exclusive`.

## Commands

Command | Action
| - | - |
exit | Exits the process.
help | Lists all available commands.
connect | Connect to remote update server. Not available if already connected to a remote server.
disconnect | Disconnect from a remote connection.
push | Overwrite the remote `shared` files with the local ones.
push-exclusive | Overwrite the remote `exclusive` files with the local ones specific to the domain.
pull | Overwrite local `exclusive` files specific to the domain with remote `exclusive` files.
sync | Runs `push`, then `push-exclusive`, then `pull` in series.
start | Starts the HTTP and SMTP servers, or the remote HTTP and SMTP servers if connected.
stop | Stops the HTTP and SMTP servers, or the remote HTTP and SMTP servers if connected.
restart | Restarts the HTTP and SMTP servers, or the remote HTTP and SMTP servers if connected.
restart-all | Restarts the application, or the remote application if connected.