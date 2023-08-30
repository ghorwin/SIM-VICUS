@echo off

pushd ..\..\data\tests

python ..\..\scripts\TestSuite\update_reference_results.py VC2019_win64

popd


