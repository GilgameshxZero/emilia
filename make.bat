call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cd src
call ".\build-id-inc.bat"
call cl ^
/EHsc ^
/Fo".\..\obj\\" ^
/Fe".\..\bin\emilia.exe" ^
/O2 ^
/Ox ^
.\main.cpp ^
.\connection-handlers.cpp ^
.\command-handlers.cpp ^
.\rain-aeternum\algorithm-libraries.cpp ^
.\rain-aeternum\gdi-plus-include.cpp ^
.\rain-aeternum\network-base.cpp ^
.\rain-aeternum\network-client-manager.cpp ^
.\rain-aeternum\network-libraries.cpp ^
.\rain-aeternum\network-recv-thread.cpp ^
.\rain-aeternum\network-server-manager.cpp ^
.\rain-aeternum\network-smtp.cpp ^
.\rain-aeternum\network-socket-manager.cpp ^
.\rain-aeternum\network-utility.cpp ^
.\rain-aeternum\network-wsa-include.cpp ^
.\rain-aeternum\rain-libraries.cpp ^
.\rain-aeternum\rain-window.cpp ^
.\rain-aeternum\utility-error.cpp ^
.\rain-aeternum\utility-filesystem.cpp ^
.\rain-aeternum\utility-libraries.cpp ^
.\rain-aeternum\utility-logging.cpp ^
.\rain-aeternum\utility-string.cpp ^
.\rain-aeternum\utility-time.cpp ^
.\rain-aeternum\windows-lam-include.cpp
cd ..