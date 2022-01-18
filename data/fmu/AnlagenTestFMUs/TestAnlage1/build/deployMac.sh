#!/bin/bash

# script is supposed to be executed in /build directory

# remove target directory if it exists
if [ -d TestAnlage1 ]; then
  rm -rf TestAnlage1 
fi &&

# remove target FMU if it exists
if [ -f TestAnlage1.fmu ]; then
    rm TestAnlage1.fmu 
fi &&

# create subdir and change into it
mkdir -p TestAnlage1 &&
cd TestAnlage1 &&

# create binary dir for Linux
mkdir -p binaries/darwin64 &&

# copy shared library, we expect it to be already renamed correctly
cp ../../bin/release/TestAnlage1.dylib binaries/darwin64/TestAnlage1.dylib &&
cp ../../data/modelDescription.xml . &&

# create zip archive
zip -r ../TestAnlage1.zip . | cat > /dev/null &&
cd .. &&
mv TestAnlage1.zip TestAnlage1.fmu &&
echo "Created TestAnlage1.fmu" &&

# change working directory back to original dir
cd -

