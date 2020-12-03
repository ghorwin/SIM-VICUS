@echo off

:: Run test suite in data/tests directory, all project files with .nandrad file extension
pushd ..\..\data\tests

set OLD_NUM_THREADS=%OMP_NUM_THREADS%

set OMP_NUM_THREADS=1
python ..\..\scripts\TestSuite\run_tests.py -p . -s ..\..\bin\release_x64\NandradSolver -e nandrad
set OMP_NUM_THREADS=%OLD_NUM_THREADS%

popd


