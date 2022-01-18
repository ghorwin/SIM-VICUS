@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\TestAnlage1.dll goto DLL_EXISTS
echo "ERROR: File TestAnlage1.dll expected in directory ..\bin\release_x64\TestAnlage1.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist TestAnlage1 goto DIRECTORY_CLEAN
echo Removing existing directory 'TestAnlage1'
rd /S /Q "TestAnlage1"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist TestAnlage1.fmu goto FMU_REMOVED
echo Removing existing FMU file 'TestAnlage1.fmu'
del /F /S /Q "TestAnlage1.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir TestAnlage1

cd TestAnlage1

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\TestAnlage1.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'TestAnlage1.zip'
cd TestAnlage1
7za a ../TestAnlage1.zip .
cd ..

echo Renaming archive to 'TestAnlage1.fmu'
rename TestAnlage1.zip TestAnlage1.fmu

:: all ok
exit /b 0
