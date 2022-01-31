@echo off

:: setup VC environment variables
set VCVARSALL_PATH="c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
call %VCVARSALL_PATH%

:: FMU-specific variables - set by code generator
set FMU_SHARED_LIB_NAME=CO2ComfortVentilation.dll

set CMAKELISTSDIR=..\..\projects\cmake

:: create and change into build subdir
mkdir bb_VC_x64
pushd bb_VC_x64

:: configure makefiles and build
cmake -G "NMake Makefiles" %CMAKELISTSDIR% -DCMAKE_BUILD_TYPE:String="Release"
nmake

if ERRORLEVEL 1 GOTO fail

popd

:: copy executable to bin/release dir
copy /Y .\bb_VC_x64\%FMU_SHARED_LIB_NAME% ..\..\..\bin\release_x64\lib%FMU_SHARED_LIB_NAME%

pause
exit /b 0

:fail
echo ** Build Failed **
exit /b 1
