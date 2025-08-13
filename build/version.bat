@REM Increments build version number at compile-time, and updates the version
@REM number into all relevant files for compilation. Created files are not
@REM ignored to preserve build "completeness", even though it is redundant.
@ECHO OFF
@REM LF newline variable.
(SET \n=^

)
SETLOCAL ENABLEDELAYEDEXPANSION

SET ROOT_DIR=%~dp0..\
SET REPO_NAME=%1

SET /P VERSION_MAJOR=<version.major.txt
SET /P VERSION_MINOR=<version.minor.txt
SET /P VERSION_REVISION=<version.revision.txt
SET /P VERSION_BUILD=<version.build.txt
SET /A VERSION_BUILD=!VERSION_BUILD!+1
<NUL SET /P=!VERSION_BUILD!!\n!> version.build.txt

@REM Create header.
<NUL SET /P=^
#pragma once!\n!^
!\n!^
#define !REPO_NAME!_VERSION_MAJOR !VERSION_MAJOR!!\n!^
#define !REPO_NAME!_VERSION_MINOR !VERSION_MINOR!!\n!^
#define !REPO_NAME!_VERSION_REVISION !VERSION_REVISION!!\n!^
#define !REPO_NAME!_VERSION_BUILD !VERSION_BUILD!!\n!> ^
version.hpp

ECHO Version !VERSION_MAJOR!.!VERSION_MINOR!.!VERSION_REVISION!.!VERSION_BUILD!.
ENDLOCAL
