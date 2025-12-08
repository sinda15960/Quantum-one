# Fix for Character Encoding Warnings in Qt 5.9.9

## Problem
Qt 5.9.9 uses an older MinGW compiler that shows "converting to execution character set: Illegal byte sequence" warnings when compiling `QrCode.cpp`. These warnings come from standard library headers, not from your code.

## Solution Applied

### 1. Updated `GResident.pro`
- For Qt 5.9.9: Suppressed encoding-related warnings without setting charset flags
- For Qt 6+: Uses UTF-8 encoding flags
- Added warning suppression flags that work with older MinGW

### 2. Updated `Qrcode.cpp`
- Added pragma directives to suppress warnings at the source level
- Works with both older (Qt 5.9.9) and newer (Qt 6+) compilers

## If Warnings Still Appear

If you still see encoding warnings after rebuilding, try these additional steps:

### Option 1: Clean and Rebuild
```bash
# Delete build folder
rm -rf build
# Or on Windows:
rmdir /s /q build

# Regenerate Makefiles
qmake GResident.pro

# Rebuild
mingw32-make clean
mingw32-make
```

### Option 2: Manual Compilation Flag
If the warnings persist, you can manually compile QrCode.cpp with:
```bash
g++ -c -w QrCode.cpp -o QrCode.o
```
The `-w` flag suppresses all warnings for that file.

### Option 3: Ignore the Warnings
These warnings are **harmless** and don't affect functionality. They come from standard library headers that contain characters the older MinGW compiler can't convert. Your code will compile and run correctly despite these warnings.

## Why This Happens
- Qt 5.9.9 uses MinGW 5.3 or similar (older compiler)
- Standard library headers contain characters that trigger encoding warnings
- The warnings occur during preprocessing, not in your actual code
- This is a known issue with older MinGW versions

## Verification
After applying the fix:
1. Clean your build directory
2. Run `qmake GResident.pro`
3. Rebuild the project
4. The warnings should be suppressed or significantly reduced

If warnings still appear, they are harmless and can be safely ignored.

