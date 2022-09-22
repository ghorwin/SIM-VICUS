@echo off
:: script is supposed to be executed in /build directory

if exist ..\bin\release_x64\CO2ComfortVentilation.dll goto DLL_EXISTS
echo "ERROR: File CO2ComfortVentilation.dll expected in directory ..\bin\release_x64\CO2ComfortVentilation.dll, but does not exist.
exit /b 1
:DLL_EXISTS

:: remove target directory if it exists
if not exist CO2ComfortVentilation goto DIRECTORY_CLEAN
echo Removing existing directory 'CO2ComfortVentilation'
rd /S /Q "CO2ComfortVentilation"
:DIRECTORY_CLEAN

:: remove target FMU if it exists
if not exist CO2ComfortVentilation.fmu goto FMU_REMOVED
echo Removing existing FMU file 'CO2ComfortVentilation.fmu'
del /F /S /Q "CO2ComfortVentilation.fmu"
:FMU_REMOVED

::create subdir and change into it
mkdir CO2ComfortVentilation

cd CO2ComfortVentilation

:: create binary dir for Windows
mkdir binaries\win64

:: copy shared library, we expect it to be already renamed correctly
xcopy ..\..\bin\release_x64\CO2ComfortVentilation.dll binaries\win64\
xcopy ..\..\data\modelDescription.xml .
echo Created FMU directory structure

::change working directory back to original dir
cd ..

::create zip archive
echo Creating archive 'CO2ComfortVentilation.zip'
cd CO2ComfortVentilation
7za a ../CO2ComfortVentilation.zip .
cd ..

echo Renaming archive to 'CO2ComfortVentilation.fmu'
rename CO2ComfortVentilation.zip CO2ComfortVentilation.fmu

:: all ok
exit /b 0
