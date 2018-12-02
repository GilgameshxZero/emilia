if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

call pre-build-event.bat
cd ../rc
call rc /Fo"..\obj\rc.res" rc.rc
cd ../src
call cl ^
/EHsc ^
/Fo"..\obj\manual\\" ^
/Fe"..\bin\emilia.exe" ^
/Fd"..\obj\manual\\" ^
/O2 ^
/Ot ^
/Ox ^
/MT ^
/MP ^
/incremental ^
..\obj\rc.res ^
.\*.cpp ^
.\rain-aeternum\*.cpp
cd ../build
call post-build-event.bat