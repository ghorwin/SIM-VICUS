#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d CoolingController ]; then
  rm -rf CoolingController 
fi &&

# remove target FMU if it exists
if [ -f CoolingController.fmu ]; then
    rm CoolingController.fmu 
fi &&

# create subdir and change into it
mkdir -p CoolingController &&
cd CoolingController &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/CoolingController.so binaries/linux64/CoolingController.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../CoolingController.zip . | cat > /dev/null &&
cd .. && 
mv CoolingController.zip CoolingController.fmu &&
echo "Created CoolingController.fmu" &&

# change working directory back to original dir
cd -

