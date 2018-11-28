@echo off
set filename=%1
set repfile=%2
:file_locked
	2>nul (
		>>%filename% echo off
	) && (
		echo %filename% writable now! writing to file
	) || (
		echo %filename% not writable; waiting...
		timeout 1
		goto :file_locked
	)
move /y %repfile% %filename%
start "Emilia" %filename% "update-restart"