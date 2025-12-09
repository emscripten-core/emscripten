Windows Python Script Launcher
==============================

This directory contains a simple launcher program for windows which is used to
execute the emscripten compiler entry points using the python interpreter.  It
uses the its own name (the name of the currently running executable) to
determine which python script to run and serves the same purpose as the
``run_python.sh`` script does on non-windows platforms.

We build this executable statically using ``/MT`` so that it is maximally
portable.
