@REM Proxy file that calls `nmake` with evaluated wildcards.
@ECHO OFF

@REM Inject `nmake` with `vcvars` if not yet available.
WHERE nmake >NUL 2>NUL
IF %ERRORLEVEL% NEQ 0 (
	CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

SETLOCAL ENABLEDELAYEDEXPANSION

@REM Pre-resolve a few wildcard variables before `nmake`.
@REM Careful to not exceed the 8192 string limit. We do not
@REM explicitly list `*.hpp` files here for that reason.
SET "NMAKE_INCL_PCH=..\rain\include\*"
FOR /F "delims=" %%I IN ('DIR /B /S /AD ..\rain\include') DO (
	SET "NMAKE_INCL_PCH=!NMAKE_INCL_PCH! %%I\*"
)
SET "NMAKE_INCL=..\include\*"
FOR /F "delims=" %%I IN ('DIR /B /S /AD ..\include') DO (
	SET "NMAKE_INCL=!NMAKE_INCL! %%I\*"
)
SET "NMAKE_PROJ_SRC=..\src\*.cpp"
FOR /F "delims=" %%I IN ('DIR /B /S /AD ..\src') DO (
	SET "NMAKE_PROJ_SRC=!NMAKE_PROJ_SRC! %%I\*.cpp"
)

@REM Suppresses CMD terminate prompt and error code.
nmake /C %* || (
	SET "LEVEL=!ERRORLEVEL!"
	CALL;
	EXIT /B !LEVEL!
)
ENDLOCAL
