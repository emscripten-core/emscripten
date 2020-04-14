@echo off
REM Entry point for running python scripts on windows systems.
REM To modify this file, edit `tools/run_python.bat` and then run
REM `tools/create_entry_points.py`

python "%~dp0\%~n0.py" %*
