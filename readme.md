# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>. This repository does not contain the front-end files, but does contain the back-end scripts beyond the servers.

See [changelog.md](changelog.md) for information on incremental updates, and [todo.md](todo.md) for information on planned updates.

## Overview

Emilia consists of three components | an HTTP server, a SMTP server, and an update server/client. All three components run under the same process. The update server is primarily used for pushing and pulling changes between development and production environments.

## Deployment Overview

There are two types of files in this project | `shared` and `exclusive`.

* `shared` | The same file should exist between all distributions of this project.
* `exclusive` | A separate version of this file should exist on each distribution of the project.

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

## Configuration Specifications

All configuration options are in the `config` directory. Unless otherwise specified, the paths below are relative to the executable, which is in .\bin\ relative to this file. A path that ends in a \ specifies a directory; otherwise, the path specifies a file. The following summarizes the options:

Option | Meaning
|-|-|
config-path | Directory containing the configuration files.
log-path | Directory containing the log files.
log-error | Filename of the log file to log errors.
log-memory | Filename of the log file to memory leaks.
log-log | Filename of the log file to log general activity with servers.
data-path | Directory containing the data files. Data files contain files which are updated and read by servers.
emilia-auth-pass | String password which is used to authenticate into this update server.
http-cgi-scripts | Configuration file where each line specifies a directory or file to serve as a CGI script.
http-custom-headers | Configuration file which specifies the headers the HTTP server will serve with its responses.
http-content-type | Configuration file which specifies the content-type header to serve for some file extensions.
http-404 | Configuration file which contains the HTML to serve upon receiving a 404.
http-server-root | Directory containing the files at the root of the HTTP server.
http-default-ctype | Upon serving a file of unknown extension, the content-type header to use.
http-default-index | Upon given a directory to serve by the HTTP server, the default file within that directory to serve.
http-transfer-buffer | Buffer size of the HTTP server.
http-cgi-timeout | Timeout of CGI scripts, in milliseconds. 0 is infinite.
smtp-users | Data file which specifies the email usernames and passwords of those registered with the SMTP server.
smtp-domain | The domain name of the SMTP server.
smtp-transfer-buffer | Buffer size of the SMTP server.
smtp-connect-timeout | Timeout of connection attempts to DNS and other SMTP servers, in milliseconds. 0 is infinite.
update-server-port | Port on which to run the update server.
update-tmp-ext | Extension to append to files which are write-locked before exiting the program to handle it.
update-transfer-buffer | Buffer size of the update server.
update-exclusive-files | Configuration file, where each line specifies a directory or file relative to the update root or the exclusive update root (depending on operation) to mark as exclusive.
update-ignore-files | Configuration file where each line specifies a directory or file relative to the update root to ignore while performing updates.
update-exc-ignore | Configuration file where each line specifies a directory or file relative to the update root or exclusive update root (depending on operation) to ignore, if that file or directory is also specified as an exclusive one.
update-exclusive-dir | Directory relative to the update root which specifies the location to keep all the domain-name organized copies of the exclusive files.
update-root | Directory relative to the executable of all the files to possibly be considered during an update.
update-script | File relative to the update root which contains the executable to run when a write conflict is detected; the executable should wait until the write conflict resolves, write to the file, then restart Emilia.