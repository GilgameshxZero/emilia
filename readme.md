# Emilia

by Yang Yan.

HTTP server, SMTP server, and basic deployment/version control system written in native C++ for the Windows platform, primarily for use on <http://emilia-tan.com>. The repository for the website front-end is at <https://github.com/GilgameshxZero/emilia-tan>.

See [changelog.md](changelog.md) for information on updates, and [todo.md](todo.md) for information on future changes.

## Project

`Emilia` serves from a `project`, defined by a local directory with a `.emilia` subdirectory. Upon startup, `Emilia` will attempt to find the closest `project` to equip. Before a project is equipped, functions associated with `Emilia`'s service will not be available. The user can also manually equip `Emilia` with a project or choose to initialize a new project in an existing directory.

### Configuration

Upon equipping a `project`, `Emilia` will also begin utilizing the `project`'s configuration options in `.emilia/config.ini`. `Emilia` has no standalone configuration.

### Index

Each `project` also contains a timestamped index at `.emilia/index.idx`. This index contains a log of all the files in the `project` at that timestamp.

## Commands

Command|Action
-|-
exit|Exits the process.
restart|Restarts the process or remote process if connected. Whichever `project` was equipped and whichever servers were running at the time of the command will be restored.
server|Start or stop running servers on the local process or a connected remote process.
connect|Connect to remote process. Not available if already connected to a remote process.
disconnect|Disconnect from a remote connection. Only available if connected remotely.
project|Switch, choose, or create and equip a new `project`.
sync|See below.

### Sync

The `sync` operation brings files from the same `project` on different machines up-to-date with each other.

1. Compare `Emilia` executables, and overwrite and restart the older one.
2. Compare `project` indexes, and choose the oldest one.
3. For each file in the `project` distributions, determine their status as either `modified` or `deleted`, and assign each a timestamp based on either the last-modified time or current time.
4. For each file in both `project` distributions, keep one version based on the following resolution table:

   A status|B status|File resolution
   -|-|-
   Modified at T1|Modified at T2 > T1|B
   None|Modified at T1|B
   Deleted|Modified at T1|Deleted
   Deleted|Deleted|Deleted

### Command line options

Option|Usage
-|-
-r|Reserved for internal use.
-p|Directory for equipped project.
-s|2-bit integer specifying which servers to start on startup. The bits from least-significant to most-significant represent the HTTP, then SMTP server (e.g. 3 specifies both servers should be started).

## Configuration

Paths which end with `\` specify a directory, and those that don't specify a file.

### Options

All configuration options are in the `config` directory. Unless otherwise specified, all paths below are resolved relative to the `project` root.

General|Usage
-|-
emilia-buffer|Buffer size of all socket operations performed.

***

HTTP|Usage
-|-
http-root|Root of the served HTTP fileserver.
http-cgi|Directories and files, relative to the root, to treat as CGI scripts for the HTTP server.
http-headers|Headers which should be served with HTTP responses.
http-content|Content-type associations with file extensions.
http-404|File, relative to the project, to serve on 404 response.
http-index|Default file to serve when a directory is requested.

***

SMTP|Usage
-|-
smtp-users|File relative to project specifying usernames and passwords of users.
smtp-domain|Domain name of SMTP domain.
smtp-to|Timeout, in MS, of connections to other SMTP servers, or 0.

***

Deployment|Usage
-|-
deploy-pw|Authentication password to connect with the deployment server.
deploy-port|Port on which to run the deployment server.
deploy-ignore|Files and directories to ignore during `sync`.

***

Logging|Usage
-|-
log-root|Directory containing the log files.
log-error|File within the logging directory for program errors.
log-memory|File within the logging directory for program memory leaks.
log-emilia|File within the logging directory for general logs.
log-http|File within the logging directory for HTTP server logs.
log-smtp|File within the logging directory for SMTP server logs.

### Multiline options

Some options above require multiple lines to specify either strings on each line or an associate map, with an option on each line. In that case, all following indented lines will be counted as part of that option.
