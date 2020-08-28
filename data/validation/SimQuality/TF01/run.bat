@echo off
set solver=..\..\..\..\bin\release_x64\NandradSolver.exe

%solver% SimQuality_TF01Var01.nandrad 
%solver% SimQuality_TF01Var02.nandrad 
%solver% SimQuality_TF01Var03.nandrad 
%solver% SimQuality_TF01Var04.nandrad 
%solver% SimQuality_TF01Var05.nandrad 
%solver% SimQuality_TF01Var06.nandrad 
%solver% SimQuality_TF01Var07.nandrad 
%solver% SimQuality_TF01Var08.nandrad 

pause
