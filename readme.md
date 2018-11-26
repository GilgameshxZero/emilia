# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>.

See [changelog.md](changelog.md) for information on incremental updates, and [todo.md](todo.md) for information on planned updates.

## Overview

Emilia consists of three components: an HTTP server, a SMTP server, and an update server/client. All three components run under the same process. Commands to the application either access the local servers, or, after connecting to a remote update server, the remote servers.

## Commands

Command | Action
| - | - |
exit | Exits the process, or, if connected to a remote udpate server, the connection.
help | Lists all available commands.
