@echo off
set PATH=c:\Qt\5.11.3\msvc2015_64\bin;%PATH%
lupdate ..\..\projects\Qt\SIM-VICUS.pro

pause

linguist SIM-VICUS_de.ts

