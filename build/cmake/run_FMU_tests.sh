#!/bin/bash

# Run test suite in data/tests directory, all project files with .nandrad file extension
export MASTERSIM_PATH=/home/ghorwin/svn/mastersim-code/bin/release/MasterSimulator
(cd ../../data/FMUTests; OMP_NUM_THREADS=1 ../../scripts/TestSuite/run_FMU_tests.py -p=./ -b=../../bin/release)

