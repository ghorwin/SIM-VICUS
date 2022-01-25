#!/bin/bash

# Run test suite in data/tests directory, all project files with .nandrad file extension
# path to MasterSimulator is expected to be set in global system or search path.
(cd ../../data/FMUTests; OMP_NUM_THREADS=1 ../../scripts/TestSuite/run_FMU_tests.py -p=./ -b=../../bin/release)

