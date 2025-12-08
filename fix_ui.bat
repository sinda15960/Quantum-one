@echo off
echo ========================================
echo Fixing UI synchronization issue
echo ========================================
echo.

echo Step 1: Cleaning old build files...
if exist build rmdir /s /q build
if exist *.o del /q *.o
if exist ui_*.h del /q ui_*.h
if exist moc_*.cpp del /q moc_*.cpp

echo.
echo Step 2: Running qmake to regenerate UI files...
qmake GResident.pro

echo.
echo Step 3: Building project...
mingw32-make clean
mingw32-make

echo.
echo ========================================
echo Done! The UI should now be synchronized.
echo ========================================
pause

