@echo off
set QTDIR=C:\Qt\5.15.2\msvc2019_64
set PATH=C:\Qt\5.15.2\msvc2019_64;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

windeployqt.exe SIM-VICUS.exe
