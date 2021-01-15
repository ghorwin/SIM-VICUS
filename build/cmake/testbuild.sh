#!/bin/bash

if [ -d "bb-gcc" ]; then
  rm -rf bb-gcc
fi &&

# create solver in bb-gcc subdir without OpenMP support
./build.sh 8 release no-gui &&

# Run test suite in data/tests directory, all project files with .nandrad file extension
(cd ../../data/tests; ../../scripts/TestSuite/run_tests.py -p ./ -s ../../bin/release/NandradSolver -e nandrad)


