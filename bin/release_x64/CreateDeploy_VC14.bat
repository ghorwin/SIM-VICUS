@echo off
set QTDIR=C:\Qt\5.11.3\msvc2015_64
set PATH=C:\Qt\5.11.3\msvc2015_64\bin;%PATH%

set VC_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"

:: setup VC environment variables
set VCVARSALL_PATH=%VC_PATH%"\vcvarsall.bat"
call %VCVARSALL_PATH% x64

copy %VC_PATH%"\redist\x64\Microsoft.VC140.CRT\msvcp140.dll" msvcp140.dll
copy %VC_PATH%"\redist\x64\Microsoft.VC140.CRT\vcruntime140.dll" vcruntime140.dll
copy %VC_PATH%"\redist\x64\Microsoft.VC140.OpenMP\vcomp140.dll" vcomp140.dll

windeployqt.exe SIM-VICUS.exe
