# Changelog

## 8.1.17

Up font-size for script in `essay.css`. Update VSCode build procedures for new `makefile`.

## 8.1.16

* Components and essay now wait for font to load before scene in.
* Add essay build task in VSCode to easily convert Markdown-generated HTML into essay HTML.
* More semantic CSS.
* Redirect localhost aliases to eutopia.

## 8.1.15

Default automation shell configuration to CMD on Windows. Update to latest rain with better Windows support. Update build procedure to match accordingly.

## 8.1.14

Reduce size of PNG on test essay.

## 8.1.13

Allow cross-origin requests from any domain for static files to allow external use of `essay.css`.

## 8.1.12

Add markdown test page. Modify some VSCode extension syntax highlight colors. Fix KaTeX rendering. Rename assets folders with `.md`. Allow forcing of light/dark in `essay.css`.

## 8.1.11

Subset font pages and reduce their size from around 3MB to around 300KB total. Re-encode `emilia` essay assets.

## 8.1.10

Fix essay side margin alphas.

## 8.1.9

Change poem (again).

## 8.1.8

Add right border to map.

## 8.1.7

Added inset shadow to a lot of elements to remove “floaty” feeling. Cache control only cahces for ~17 minutes by default instead of 1 week.

## 8.1.6

Modify favicon to sunflower. Still need to custom draw a sunflower icon. Remove some unnecessary CSS.

## 8.1.5

Mostly finish work on “erlija-past”. Still some `.gif` to `.webm` re-encodes to do, and some cleaning up.

## 8.1.4

Begin redesign on “erlija-past” theme.

## 8.1.3

Lower outbox retry attempts to 8. Increase default retry time to 4h. Fix outbox retry failure detection. Only display emails from the last 72 hours on the status page. On debug builds, allow any host header to match eutopia filter.

## 8.1.2

Update `build/makefile` to use `clang++` by default and not overwrite `CXX` environment variable.
Reset default ports to `0`.
Previous design redirects don’t use HTTPS anymore.

## 8.1.1

Update to `rain 7.1.1`. Future updates to the underlying `rain` will not be so immediate. This introduces no changes to `emilia`.

## 8.1.0 “Eutopia”

This update brings all the new changes from `rain 7.0` and `7.1` in addition to re-incorporating all previous major redesigns and setting up for a future redesign by wiping the main site. Repositories `emilia-mail` and `emilia-tan` consisting of the SMTP server and front-end for the HTTP server have been merged into this repository, and `rain` is now configured as a submodule. The umbrella repository of `emilia` is retired and this repository is to take on the name `emilia` from now on.

Four previous major designs have been incorporated into the HTTP server:

* `hyperspace`, from version `2.1.2` (since version `1.0.0` and prior) at commit `1614dd58128dc673c97c11e2f26ced9f8c74d65f`.
* `hyperpanel` (named retroactively), from version `3.1.3` at commit `836ba69190ef2dc4813ff1f0404cfcaf01b2c36d`.
* `pastel`, from version `4.0.0` at commit `b40b81d75e9730aa8ba7a17688a8a6ab5c302a7e` from the retired `emilia-tan`.
* `starfall`, from version `5.0.8` (through version `7.1.10`) at commit `25252431fc5510d04d9a58f74c76e1cb7ce7a974` from the retired `emilia-tan`.

These past designs are served as subdomains at their respective names.

In addition, the current design-in-progress, is served at the optional subdomain `eutopia`. The subdomain `status` is reserved for plain server information. Since the SMTP and HTTP servers are now combined into one process, information from the SMTP server will also be shown on the status page.

Additional command-line commands as well as runtime commands have been implemented to fine-tune the operation of the servers. Shared mutexes are utilized for thread-safety on mostly-read objects. A data consistency/database object is in development for future versions, once the `eutopia` redesign is finalized through the revisions on subversion `8.1`.

## 8.0.2

SMTP mailbox activity is now accessible via the status page.

## 8.0.1

Fix a major bug where server accept threads would busy-wait once the initial accept timeout expired.

## 8.0.0

Incorporated both HTTP and SMTP servers together as well as the “Emancipation” `rain 7.0` release updates.

## 7.1.10

* Update the host node config to match `rain v6.5.4`.

## 7.1.9

* Update to the `getLength` function and `char const**` type for `Generator` in `Rain::Networking` servers.
* Modify VSCode build tasks to create `bin` and `obj` folders automatically, better.

## 7.1.8

* Makefile now links libraries after objects as intended.

## 7.1.7

* Additional small fixes for include paths and versioning.

## 7.1.6

* Use `-g` and `-march=native` flags in compilation like in the `rain` project.

## 7.1.5

* Remove “An Amazing Project” from readme.

## 7.1.4

* Revamp `makefile` to pass correct linker and compiler flags.
* Setup Visual Studio Code debugging workflows.

## 7.1.3

* Fix a bug where 404 `Content-Length` was not being sent correctly.

## 7.1.2

* Fix `make noinc`.
* Don't sort `using` in `.clang-format`.
* Use `rain v6.4.0`.

## 7.1.1

* Update `noinc` make command.

## 7.1.0

* Fix the RAII `Socket` implementation and update inheritance model to match.
* Fix `MacOS` builds.

## 7.0.14

* Update to RAII implementation of `Rain::Socket`.
* Add additional `make` options and switch to WSL from Cygwin.

## 7.0.13

* Shorten default timeouts.

## 7.0.12

* Update to use the new templates for `ServerSlave` with custom data types.

## 7.0.11

* Fix a bug where the file cache was wiped on every new request. This should improve performance for smaller files and minimize disk reads.

## 7.0.10

* Use `-lstdc++fs` flag for `g++` to support `g++-8`.
* Add `data` directory to version control.

## 7.0.9

Using `rain 6.0.20`, implement similar server functionality to `emilia-web 6.x.x`.

## 7.0.8

Modify makefile to increment build number correctly. Remove dependency temporarily on higher-order functionality of `rain` while `rain` is undergoing refactoring.

## 7.0.7

Change makefile to recompile when `rain` headers are modified.

## 7.0.6

Asynchronous server now parses incoming requests and prints them.

## 7.0.5

Create an asynchronous server with `ThreadPool` and "handle" incoming socket connections.

## 7.0.4

The simplest web server which returns a 200 saying "i love you" regardless of the request.

## 7.0.3

Switch to header-only `rain`. Use `HttpSocket` to setup a server and client. Piggyback off of shared code for property files in Visual Studio project.

## 7.0.2

Implement basic example for socket client.

## 7.0.1

Implemented basic command line parsing via `Rain::CommandLineParser`.

## 7.0.0

Setup cross-platform build and separate SMTP server from HTTP server of `emilia`.
