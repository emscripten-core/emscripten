@echo off
:: Entry point for running python scripts on windows systems.
:: To modify this file, edit `tools/run_python.bat` and then run
:: `tools/create_entry_points.py`

:: Using setlocal here means that our use of PYTHON variable
:: won't effect the calling shell.
setlocal

if "%PYTHON%"=="" (
  set PYTHON=%EMSDK_PYTHON%
)

if "%PYTHON%"=="" (
  set PYTHON=python
)

call "%PYTHON%" "%~dp0\%~n0.py" %*

endlocal
