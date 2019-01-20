set filename=%1

if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

call pre-build-event.bat
cd ..\root\src

if "%~2"=="aeternum" (
    call cl ^
    /EHsc ^
    /Fo"..\..\obj\scripts\\" ^
    /Fe"..\scripts\%filename%.exe" ^
    /Fd"..\..\obj\scripts\\" ^
    /O2 ^
    /Ot ^
    /Ox ^
    /MT ^
    /MP ^
    /incremental ^
    ..\..\src\rain-aeternum\*.cpp ^
    "%filename%.cpp"
) ELSE (
    call cl ^
    /EHsc ^
    /Fo"..\..\obj\scripts\\" ^
    /Fe"..\scripts\%filename%.exe" ^
    /Fd"..\..\obj\scripts\\" ^
    /O2 ^
    /Ot ^
    /Ox ^
    /MT ^
    /MP ^
    /incremental ^
    "%filename%.cpp"
)

cd ..\..\build
call post-build-event.bat