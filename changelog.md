# Changelog

## 8.5.5

1. Removed console output for snapshot refreshes.

## 8.5.4

1. Add POST endpoint for snapshot refreshing.

## 8.5.3

1. Performance: increase HTTP buffer size to 2^16 from 2^10.
2. Performance: directly send body from `std::filebuf` instead of first parsing to `std::stringbuf`.

## 8.5.2

1. Add API support for submodule snapshots on the FE.

## 8.5.1

1. Moved snapshot refresh from an endpoint to a command.

## 8.5.0

1. Remove storyworld routing from backend and move to frontend.
2. Add `/api/refresh` to refresh snapshot tags.
3. Use shared/exclusive lock framework for outbox performance.
4. Add endpoint for snapshot retrieval by tag.
5. Rework frontend in `echidna` to be mobile-responsive and updated the `erlija-past` color theme to be more saturated.
6. Reworked frontend components and FOUC resolutions.
7. Temporarily disabled dark theme `reflections-on-blackfeather`.

## 8.4.7

1. Fix compilation error.

## 8.4.6

1. Fix `api/outbox.json` to output actual JSON.

## 8.4.5

1. Remove submodules from `gitignore`.
2. Rebase `echidna` on a new history without large blobs.

## 8.4.4

1. Add both submodules to the `.gitignore`.

## 8.4.3

1. Re-add `rain` and `echidna` submodule. Next update, these two directories will be added to the `.gitignore` to enable local symlinking.

## 8.4.2

1. Temporarily de-init all submodules in preparation for submodule re-org.

## 8.4.1

1. Prepare outbox endpoint.
2. Formalize ping module.

## 8.4.0

1. Reworked endpoint policy to allow for more straightforward cache policy.
2. Updated to latest `rain`.
3. Added `api/ping` endpoint and minimal `dashboard` user-facing endpoint.
4. Should now reject SMTP emails which do not send any data.

## 8.3.15

Remove deprecated `terminal.integrated.automationShell.windows` from `settings.json`.

## 8.3.14

Update `snapshots`; add prompt to continue from `erlija-past` `splash`.

## 8.3.13

Update `snapshots`; remove `snapshot` build routines from `emilia`.

## 8.3.12

Add `snapshots` submodule.

## 8.3.11

Remove `static/snapshots` in preparation for making `snapshots` a submodule.

## 8.3.10

* Forced paths now request the right API endpoint for default theme.
* Using short default-noreferrer link in `homochromatic-square-perimeters` now.

## 8.3.9

Small bugfix to storyworld selector cookie path.

## 8.3.8

Fix small typos in `homoochromatic-square-perimeters`.

## 8.3.7

Complete the writeup `homoochromatic-square-perimeters`.

## 8.3.6

Small frontend hotfixes.

## 8.3.5

Added `felysian-blackbird`.

## 8.3.4

Animations hotfix on mobile.

## 8.3.3

Small front-end changes.

## 8.3.2

Mobile-centric frontend fixes.

## 8.3.1

Enable linking to pages in `reflections-on-blackfeather`, as well as update `Makefile` to use `snapshots` instead of `essay`.

## 8.3.0

Added a new theme/storyworld `reflections-on-blackfeather` which is now the default dark storyworld. Revamped frontend themes to allow for theme switching and reasonable defaulting & forcing. Still WIP to make this system robust.

Added documentation for backend enabling this in `http.hpp`.

## 8.2.8

Remove HTML essays from version control and add corresponding build routines in VSCode.

## 8.2.7

List all SMTP activity in `/status`.

## 8.2.6

* Remove "competitive programming" from `erlija-past` map marker.

## 8.2.5

* Increase server & client timeouts from 15 to 60 seconds, since some SMTP servers are slow to respond initially (e.g. `tormails.com`). This may solve the bug where select emails from `ghost.exponentialview.com` and `mg2.substack.com` do not receive any Envelope data, but is pending testing.
* Remove incorrectly updated `attemptSystemTime` in `Envelope`, instead calculating the `system_clock::time_point` dynamically upon hitting the relevant HTTP endpoint. This solves the incorrect SMTP timestamp sorting order bug on the `/status` endpoint.
* Check for sendable PENDING envelopes both before and after waiting on the `outboxEv` solving the bug of sometimes delaying sendable PENDING envelopes until the next trigger of `outboxEv`.
* Switched SMTP client to `EHLO` and returned additional `8BITMIME` and `SMTPUTF8` extensions on receiving a `EHLO` in server. This allows support for Unicode/international mailboxes.
* Use `_` instead of `/` while base-64 encoding from and to address for envelope data filenames to avoid invalid filenames containing `/`.

## 8.2.4

Updated rain to `7.1.19`. Modified square frames writeup to remove speculative algorithms.

## 8.2.3

Initial writeup for square frames.

## 8.2.2

Added a poem.

## 8.2.1

* Update lore.

## 8.2.0

* Remove legacy frontends.
* Update `rain` to latest.
* Refactor build routine into unified system-independent makefile.
* Build theme-switching frontend framework.

## 8.1.22

* Case-insensitive SMTP domain matching. HTTP host matching is still case-sensitive by default and dependent on specified regex, which is specified from the `domain` command-line parameter.
* Silent reveal for select VSCode tasks.
* Darker color for essay links.

## 8.1.21

Update lore.

## 8.1.20

Implement hash-location for essays & a functioning back button, and fix the content-type for `.webm`.

## 8.1.19

Update lore.

## 8.1.18

Update essay max-width to `80ch` including paddings.

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
