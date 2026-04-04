# Windows Python Script Launcher

This directory contains a simple launcher program for Windows which is used to
execute the Emscripten compiler entry points using the python interpreter. It
uses the its own name (the name of the currently running executable) to
determine which python script to run and serves the same purpose as the shell
scripts (see `run_python.sh` ) do on non-Windows platforms.

## Building

The primary `build.bat` script uses MSVC and links against `ucrtbase.dll` which
ships as part of the OS since Windows 10 (2015) and is available via
Windows Update for Vista/7/8/8.1.

`build.sh` cross-compiles with MinGW and links against `msvcrt.dll` (present
on all Windows versions). This script is mostly useful for debugging on Linux.
The .exe file that is checked in here, and used in by emsdk is currently always
built with the `build.bat` script above.

## Related projects

### posy-trampoline

The posy-trampoline project does something similar:

    https://github.com/njsmith/posy/tree/main/src/trampolines/windows-trampolines/posy-trampoline

However, IIRC it also embeds the target python file onto the executable itself
which is not some Emscripten needs (or wants) to do.  Its also written in rust
(which is not something Emscripten has in any of its dependencies yet).
The `uv` tool also embeds a version of this trampiline:

    https://github.com/astral-sh/uv/tree/main/crates/uv-trampoline

There is also PyInstaller (https://pyinstaller.org/en/stable/), but that seems
to want to wrap up the whole application into the executable too.

Both these projects seem to change the way the python code itself is delivered
to Windows users.  The difference with Emscripten's pylauncher is that
it leaves the layout of the python files unchanged.  i.e. Its just a launcher,
not any kind of bundler.  This means that Windows users can still see, and even
modify in place, all the python files just like non-Windows users.  All the
launcher does is allows the entry point to be an `.exe`.

The Emscripten launcher is also very small, coming it at just 5.5k at time of
writing.

The Emscripten launcher also doesn't require any modification of the `.exe` to
deploy it.  All one needs to do is copy the unmodified executable alongside a
python file of the same name, and the launcher will find it and run it based
purely on the name of the launcher itself (i.e. argv0).

### setuptools

There is windows launcher that is part of setuptools:

    https://github.com/pypa/setuptools/blob/main/launcher.c

The code is very similar to Emscripten's launcher.  In fact, perhaps we should
consider switch it this in the future?

Unlike Emscripten's laucnher this launcher seems to look for
`<exe_basename>-script.py` rather than just `<exe_basename>.py`, which is what
the Emscripten laucnher uses.

This laucnher also seem to examine the `#!` line of the target script and
locate the python executable somehow based on this.
