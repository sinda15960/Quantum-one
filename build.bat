@echo off
cd /d "c:\Users\user\Desktop\EmployéCrud+Intégration"
if not exist debug mkdir debug
"C:\Qt\6.7.3\mingw_64\bin\qmake.exe" GResident.pro
"C:\Qt\Tools\mingw1120_64\bin\mingw32-make.exe" -f Makefile.Debug -j4
echo.
echo Build Complete!
pause
