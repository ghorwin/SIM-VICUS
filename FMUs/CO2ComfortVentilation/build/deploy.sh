#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d CO2ComfortVentilation ]; then
  rm -rf CO2ComfortVentilation 
fi &&

# remove target FMU if it exists
if [ -f CO2ComfortVentilation.fmu ]; then
    rm CO2ComfortVentilation.fmu 
fi &&

# create subdir and change into it
mkdir -p CO2ComfortVentilation &&
cd CO2ComfortVentilation &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/CO2ComfortVentilation.so binaries/linux64/CO2ComfortVentilation.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../CO2ComfortVentilation.zip . | cat > /dev/null &&
cd .. && 
mv CO2ComfortVentilation.zip CO2ComfortVentilation.fmu &&
echo "Created CO2ComfortVentilation.fmu" &&

# change working directory back to original dir
cd -

