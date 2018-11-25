cd src
call "build-id-inc.bat"
call rc /Fo"..\obj\rc.res" rc.rc
call cl ^
/EHsc ^
/Fo"..\obj\\" ^
/Fe"..\bin\emilia.exe" ^
/Fd"..\obj\\" ^
/O2 ^
/Ot ^
/Ox ^
/MT ^
/MP ^
/Zi ^
/Z7 ^
/Zl ^
/incremental ^
..\obj\rc.res ^
.\*.cpp ^
.\rain-aeternum\*.cpp
cd ..