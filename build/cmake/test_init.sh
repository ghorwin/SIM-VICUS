#!/bin/bash

# Run test suite in data/tests directory, all project files with .nandrad file extension
(cd ../../data/tests/NetworkStaticFlow; OMP_NUM_THREADS=1 ../../../scripts/TestSuite/run_tests.py -p ./ -s ../../../bin/release/NandradSolver -e nandrad --test-init)

