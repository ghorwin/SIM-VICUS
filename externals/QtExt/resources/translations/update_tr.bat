@echo off
set QTDIR=C:\Qt\Qt5.11.3\5.11.3\msvc2015
set PATH=C:\Qt\Qt5.11.3\5.11.3\msvc2015\bin;%PATH%

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
call %VCVARSALL_PATH%

rem 1. Call this batch file to update the ts files.
rem 2. Then edit the ts files with Qt Linguist
rem 3. Then call release_tr.bat to generate release files and copy them to the bin/IBK/resources/translations directory.
lupdate ..\..\projects\Qt\QtExt.pro


echo Translation files (*.ts) updated, use Qt Linguist to add missing translations!
PAUSE

linguist QtExt_de.ts
