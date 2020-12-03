@echo off

pushd ..\..\data\tests

python ..\..\scripts\TestSuite\update_reference_results.py VC14_win64

popd


