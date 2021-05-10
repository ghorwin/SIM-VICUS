@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\CoolingController.dll goto DLL_EXISTS
echo "ERROR: File CoolingController.dll expected in directory ..\bin\release_x64\CoolingController.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist CoolingController goto DIRECTORY_CLEAN
echo Removing existing directory 'CoolingController'
rd /S /Q "CoolingController"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist CoolingController.fmu goto FMU_REMOVED
echo Removing existing FMU file 'CoolingController.fmu'
del /F /S /Q "CoolingController.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir CoolingController

cd CoolingController

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\CoolingController.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'CoolingController.zip'
cd CoolingController
7za a ../CoolingController.zip .
cd ..

echo Renaming archive to 'CoolingController.fmu'
rename CoolingController.zip CoolingController.fmu

:: all ok
exit /b 0
