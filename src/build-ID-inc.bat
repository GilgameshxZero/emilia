@echo off
setlocal enabledelayedexpansion
set filename=build-id.h
if exist "%filename%.temp" del "%filename%.temp"
for /F "tokens=*" %%R in (%filename%) do (
	if "%%R"=="" echo. >> "%filename%.temp"
	set incflag=0
	for /f "tokens=1-3 delims= " %%A in ("%%R") do (
		if "%%B"=="VERSION_BUILD" set incflag=1
		if !incflag! equ 1 (
			set /a num=%%C+1
			echo %%A %%B            !num!>> "%filename%.temp"
		) else (
			echo %%R>> "%filename%.temp"
		)
	)
)
del "%filename%"
ren "%filename%.temp" "%filename%"