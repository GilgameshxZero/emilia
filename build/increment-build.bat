@ECHO OFF

REM Increments build version at compile-time.

SETLOCAL ENABLEDELAYEDEXPANSION
SET versionpath=%~dp0..\include\build.hpp
FOR /F "tokens=1-3 delims= " %%A IN (%versionpath%) DO (
	SET /A versionbuild=%%C+1
	ECHO | SET /P="%%A %%B !versionbuild!" > %versionpath%
)
ECHO build !versionbuild!
ENDLOCAL
