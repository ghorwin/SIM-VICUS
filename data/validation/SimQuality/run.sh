#!/bin/bash

# simulation all nandrad projects with current solver
python ../../../scripts/TestSuite/run_tests.py --run-all -s ../../../bin/release/NandradSolver -p . -e nandrad
