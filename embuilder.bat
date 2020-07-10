@echo off
:: Entry point for running python scripts on windows systems.
:: To modify this file, edit `tools/run_python.bat` and then run
:: `tools/create_entry_points.py`

:: emsdk will set EMSDK_PYTHON, otherwise just look for python in PATH
@IF "%EMSDK_PYTHON%"=="" (
  @SET EMSDK_PYTHON=python
)

%EMSDK_PYTHON% "%~dp0\%~n0.py" %*
