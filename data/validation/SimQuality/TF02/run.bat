@echo off

set solver=..\..\..\..\bin\release_x64\NandradSolver.exe

%solver% SimQuality_TF02.nandrad

%solver% SimQuality_TF02_Perez.nandrad

pause
