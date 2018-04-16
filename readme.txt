Emilia-tan

EMTServer: Basic webserver to respond to HTTP requests.
EMTSMTPClient: Internally-used application to send emails through SMTP.
EMTSMTPServer: Basic SMTP server to respond to incoming emails in SMTP.

Changelog:
	EMTServer
		todo
			clean up memory leaks
		3.7.1
			added .py content-type spec
		3.7.0
			updated fileversion detail on .exe
			standardized logging more
			added header "connection: keep-alive" support
			increased default buffer sizes
		3.6.0
			standardized Server application flow into Rain Libraries, and refactored code accordingly
		3.5.1
			updated content-type setting for .mkv files to video/webm
			light refactoring
			project structure changes
		3.5.0
			added append option in error output
			standardized some logging
			now buffered socket communications from server

	EMTSMTPClient
		todo
			standardize to new rain library
		1.1.1
			light refactoring
			project structure changes
			updated fileversion detail on .exe

	EMTSMTPServer
		todo
			standardize to new rain library
		1.2.1
			light refactoring
			project structure changes
			updated fileversion detail on .exe