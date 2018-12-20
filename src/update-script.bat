@echo off
set server=%1
set src=%2
if "%~3"=="" (
	set dest=%server%
	echo no destination found; replacing %server% instead
) else (
	set dest=%3
)
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
rem allow time for concurrent update scripts to finish
timeout 1
start "Emilia" %server% "update-restart"