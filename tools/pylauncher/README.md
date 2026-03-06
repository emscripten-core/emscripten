# Windows Python Script Launcher

This directory contains a simple launcher program for windows which is used to
execute the emscripten compiler entry points using the python interpreter. It
uses the its own name (the name of the currently running executable) to
determine which python script to run and serves the same purpose as the
`run_python.sh` script does on non-windows platforms.

## Building

The primary `build.bat` script uses MSVC and links against `ucrtbase.dll` which
ships as part of the OS since Windows 10 (2015) and is available via
Windows Update for Vista/7/8/8.1.

`build.sh` cross-compiles with MinGW and links against `msvcrt.dll` (present
on all Windows versions). This script is mostly useful for debugging on Linux.
The .exe file that is checked in here, and used in by emsdk is currently always
built with the `build.bat` script above.
