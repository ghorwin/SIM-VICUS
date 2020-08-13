#!/bin/bash

# create solver in bb-gcc subdir
./build.sh 8 release no-gui &&

# Run test suite in data/tests directory, all project files with .nandrad file extension.
OMP_NUM_THREADS=1 ../../scripts/TestSuite/run_tests.py -p ../../data/tests -s ./bb-gcc/NandradSolver/NandradSolver -e nandrad


