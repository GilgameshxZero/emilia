# Emilia-tan
Monochrome code: 3

## About

### Author
Yang Yan, USACO Finalist & MIT 2021.

### Purpose
SMTP & HTTP server written in native C++ for the Windows platform, primarily for use on emilia-tan.com. The repository includes some scripts and files hosted on the emilia-tan.com domain.

### Conformity
Emilia-tan conforms to RFC protocol as much as possible, but not completely, as it is still work-in-progress.

## Workflows

### Development
There are four types of files: auxiliary, configuration, code, and server.
* Auxiliary: Files generated by server, such as logs. This can contain sensitive information.
* Code: Server executables & helpers. Should not include sensitive information.
* Configuration: Files which provide options for the server; different between development & production. This may include some sensitive information. Also includes update server and client scripts.
* Server: Files served by the server, including databases. Will contain sensitive information.

In addition, there are archived files, from previous version of the repository.

Files are organized in any development root as follows:
* Development: Files currently in development. The files under development are not guaranteed to be functional at any point in time.
	* Auxiliary
	* Code
		* EmiliaSite: HTTP/HTTPS webserver.
		* EmiliaMail: SMTP client and server.
		* EmiliaUpdate: Update server/client.
		* Common: Shared code between all projects, such as the Rain Library
	* Configuration
	* Server
* Staging: Intermediary between development and production. Functional most of the time.
	* Auxiliary
	* Code: Any code files, in the same structure as in production.
	* Configuration: Any configuration files, same as in production.
	* Server
* Production: Files currently in production. guaranteed to be functional at all times, as well as identical to production at all times if the update script is running.
	* Auxiliary
	* Code
	* Configuration
	* Server

### Updating & Deploying (to AWS e.g. emilia-tan.com, with e.g. EmiliaUpdate)
Deployment should be non-continuous. The production server will have an update server. Development will run an update client which connects with the server update script and transfers any changes. This way, we can avoid having more private information accesible through github while granting full control over deployment.

There exists a staging stage between development and production. Staging will need to be functional before deployment to production.

Files are organized on production exactly as in the /Production directory.

The update server/client will be referred to as the update script. When updating, the update client in **staging** should always be used. The update server should be run from **production**. Take note that even as the update process is underway, production files may change, likely the auxiliary and server files. There are several functions the script communication should complete:
* Development to Staging: Updates /Staging with /Development as follows:
	* Auxiliary: Not modified.
	* Code: Maintains directory structure with only relevant files inside; **relevant files are determined in a client-side configuration file for the update script, under staging**, from where the udpate script shall be run. The update script may also update itself in this step with the script from production.
		* If the script itself is specified as a relevant staging file, then, the script should run another executable, which waits until the current executable exits to do the replace operation, and then restart the script.
	* Configuration: Not modified.
	* Server: Not modified.

Before deployment from staging to production, any necessary configuration and server file changes should be made in staging. Staging will inevitably modify files in production that production also modifies. We have an ignore list in the udpate client configuraiton which can prevent merge conflicts.
* Staging to Production: Before running this command, the staging files should be identical to those to be used in production. This command will do a few things in the following order:
	* Shut down production.
	* Download production files to /Production.
	* Wipe /Production and replace with /Staging, **ignoring files specified in the configuration for the update client in staging**.
		* Ignored files should be auxiliary, configuration, or server, but never code.
	* Replace /Staging with /Production, without ignoring any files.
	* Restart the update script.
		* Likely, the update script and its configurations should not be ignored in updating /Production, so this step is not necessary most of the time. However, restarting may be necessary if we wish to replace the executable.
	* Upload /Production to production.
	* Start production.

There are a few additional functions for ease of use:
* Production Download: Changes to production files (likely auxiliary and server) will be reflected in the development environment under /Production.
* Production Stop: Stops production, by executing commands specified in the update script configuration.
* Production Start: Starts production, again by executing a set of commands specified in configuration.
* Production Sync Start: Actively update /Production with any changes to production. While sync is active, commands other than Production Sync Stop cannot be executed.
* Production Sync Stop: Stops the active sync.

### Versioning
Each program under `Code` will have a separate version associated with it. THe format is the same, as follows: `major.minor.revision.build`. The main repository will have a version associated with it as well, but without `build`.

The changelog for the versions will be in this readme document.

### Version Control & Github
Only the following directory structures will be version controlled, primarily for privacy reasons:
* Development
	* Auxiliary
	* Code
	* Configuration
	* Server
* Staging
	* Code
* Production
	* Code

Git commits to master encouraged to contain updated versioning of each component of the repository.

## Additional Files
In the root, there will also be additional untracked files not part of the official repository.
* emilia-tan.com: Files specific to emilia-tan.com's management

## Changelog

### Emilia-tan
* 1.2.0
	* (EmiliaSiteServer, EmiliaMailClient, EmiliaMailServer, EmiliaUpdateClient, EmiliaUpdateServer, EmiliaUpdateCRHelper) = (3.7.2, 1.1.1, 1.2.2, 1.0.0, 1.0.0, 1.0.0)
* 1.1.1
	* (EmiliaSiteServer, EmiliaMailClient, EmiliaMailServer, EmiliaUpdateClient, EmiliaUpdateServer) = (3.7.2, 1.1.1, 1.2.2, -, -)
	* removed .vs from git
* 1.1.0
	* (EmiliaSiteServer, EmiliaMailClient, EmiliaMailServer, EmiliaUpdateClient, EmiliaUpdateServer) = (3.7.2, 1.1.1, 1.2.2, -, -)
	* established Development, Staging, Production branches
		* Production maintained with live production environment
		* Staging maintained with in dev code
		* old projects re-organized into Development in preparation for EmiliaUpdate and staging and production deployment
* 1.0.0
	* established new workflows for staging, production, and deployment
	* introduced EmiliaUpdate
	* modified names
		* EMTServer -> EmiliaSite
		* EMTSMTPServer/EMTSMTPClient -> EmiliaMail
	* hotfixed EmiliaMail and EmiliaSite using old workflows
	* hotfixed Github repository.

### EmiliaSite

#### EmiliaSiteServer
* 3.7.2
	* fixed POST request header length parsing
	* cleaned up resource.h
* 3.7.1
	* added .py content-type spec
* 3.7.0
	* updated fileversion detail on .exe
	* standardized logging more
	* added header "connection: keep-alive" support
	* increased default buffer sizes
* 3.6.0
	* standardized Server application flow into Rain Libraries, and refactored code accordingly
* 3.5.1
	* updated content-type setting for .mkv files to video/webm
	* light refactoring
	* project structure changes
* 3.5.0
	* added append option in error output
	* standardized some logging
	* now buffered socket communications from server

### EmiliaMail

#### EmiliaMailClient
* 1.1.1
	* light refactoring
	* project structure changes
	* updated fileversion detail on .exe

#### EmiliaMailServer
* 1.2.2
	* cleaned up resource.h
	* hotfixed RainLibrary3 links
	* changed some parameters to the client
* 1.2.1
	* light refactoring
	* project structure changes

### EmiliaUpdate

#### EmiliaUpdateClient
* 1.0.0
	* Development to Staging
	* integration with CRH

#### EmiliaUpdateCRHelper
* 1.0.0
	* allows a copy + run operation delayed until the destination is writable

#### EmiliaUpdateServer
* 1.0.0
	* 

## Todo

### Emilia-tan
* implement a testing framework for both development and staging.
* implement a monitoring framework for production.

### EmiliaSite

#### EmiliaSite
* clean up memory leaks

### EmiliaMail

#### EmiliaMailServer
* standardize to new rain library

#### EmiliaMailClient
* standardize to new rain library

### EmiliaUpdate

#### EmiliaUpdateClient

#### EmiliaUpdateServer