#include "constants.hpp"

namespace Emilia {
	const int MAX_PROJECT_DIR_SEARCH = 1000;
	const std::string RESTART_SHELL_SCRIPT = R"(@echo off
set server=%1
set src=%2
set dest=%3
set r=%4
set p=%5
set s=%6
echo %r%
echo %p%
echo %s%
:file_locked
	2>nul (
		>>%dest% echo off
	) && (
		echo %dest% writable now! writing to file
	) || (
		echo %dest% not writable; waiting...
		timeout 1
		goto :file_locked
	)
move /y %src% %dest%
rem allow time for concurrent deploy scripts to finish
timeout 1
start %server% %server% -r %r% -p %p% -s %s%
rem allow for ConEmu to close tab
timeout 10
(goto) 2>nul & del "%~f0"
)", //the shell script created the replace the executable and run a new one
DEFAULT_CONFIGURATION = R"(emilia-buffer		65536

http-root			root\
http-cgi
	root\scripts\
http-cgi-to		0
http-headers
	server							Emilia
	server-version
	content-disposition	inline
	content-range
	accept-ranges				bytes
http-content
	aac					audio/aac
	bin					application/octet-stream
	css					text/css
	csv					text/csv
	doc					application/msword
	docx				application/vnd.openxmlformats-officedocument.wordprocessingml.document
	epub				application/epub+zip
	flac				audio/flac
	gif					image/gif
	htm					text/html
	html				text/html
	ico					image/x-icon
	ics					text/calendar
	jar					application/java-archive
	jpeg				image/jpeg
	jpg					image/jpeg
	js					application/javascript
	json				application/json
	md					text/markdown
	mkv					video/webm
	mid					audio/midi
	midi				audio/midi
	mpeg				video/mpeg
	mpkg				application/vnd.apple.installer+xml
	mp3					audio/mpeg
	mp4					video/mp4
	oga					audio/ogg
	ogg					audio/ogg
	ogv					video/ogg
	otf					font/otf
	pdf					application/pdf
	png					image/png
	ppt					application/vnd.ms-powerpoint
	pptx				application/vnd.openxmlformats-officedocument.presentationml.presentation
	py					text/x-python
	rar					application/x-rar-compressed
	rtf					application/rtf
	sh					application/x-sh
	svg					image/svg+xml
	swf					application/x-shockwave-flash
	tar					application/x-tar
	tif					image/tiff
	tiff				image/tiff
	ts					application/typescript
	ttf					font/ttf
	txt					text
	vsd					application/vnd.visio
	wav					audio/x-wav
	weba				audio/webm
	webm				video/webm
	webp				image/webp
	woff				font/woff
	woff2				font/woff2
	xhtml				application/xhtml+xml
	xls					application/vnd.ms-excel
	xlsx				application/vnd.openxmlformats-officedocument.spreadsheetml.sheet
	xml					application/xml
	zip					application/zip
	7z					application/x-7z-compressed
http-404			root\404.html
http-index		index.html
http-port			80

smtp-users		smtp-users.ini
smtp-domain		emilia-tan.com
smtp-to				10000
stmp-port			25

log-root			logs\
log-error			errors.log
log-memory		memory-leaks.log
log-general		general.log
log-http			http.log
log-smtp			smtp.log
log-deploy		deploy.log

deploy-pw			password
deploy-port		50368
deploy-ignore
	.git\
	logs\)",
		PROJECT_DIR = ".emilia\\",
		PROJECT_INDEX = PROJECT_DIR + "index.idx",
		PROJECT_CONFIG = PROJECT_DIR + "config.ini";
}