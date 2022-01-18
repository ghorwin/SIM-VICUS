#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d HeatingModel ]; then
  rm -rf HeatingModel 
fi &&

# remove target FMU if it exists
if [ -f HeatingModel.fmu ]; then
    rm HeatingModel.fmu 
fi &&

# create subdir and change into it
mkdir -p HeatingModel &&
cd HeatingModel &&

# create binary dir for Linux
mkdir -p binaries/linux64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/HeatingModel.so binaries/linux64/HeatingModel.so &&
cp ../../data/modelDescription.xml . &&

# create zip archive
7za a ../HeatingModel.zip . | cat > /dev/null &&
cd .. && 
mv HeatingModel.zip HeatingModel.fmu &&
echo "Created HeatingModel.fmu" 

mv -f HeatingModel.fmu ../../HeatingModel.fmu
