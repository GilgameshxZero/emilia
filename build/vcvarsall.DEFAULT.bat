@IF NOT DEFINED _ECHO ECHO OFF

@REM Calls the vcvarsall.bat file from the local Visual Studio installation to load relevant environment variables.
@REM If the path is incorrect for your system, copy this file to the gitignored "vcvarsall.bat" and replace the path there.

SET VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"
IF EXIST %VCVARSALL_PATH% (
	CALL %VCVARSALL_PATH% %*
)
