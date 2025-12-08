@echo off
cd /d "c:\Users\user\Desktop\EmployéCrud+Intégration"
echo Cleaning build artifacts...
if exist debug rmdir /s /q debug
if exist release rmdir /s /q release
mkdir debug

echo Running qmake...
"C:\Qt\6.7.3\mingw_64\bin\qmake.exe" GResident.pro

echo Building project...
"C:\Qt\Tools\mingw1120_64\bin\mingw32-make.exe" -f Makefile.Debug -j4

echo.
if exist debug\GResident.exe (
    echo SUCCESS: GResident.exe built successfully!
    echo Output: %cd%\debug\GResident.exe
) else (
    echo Build completed, checking for executable...
)
pause
