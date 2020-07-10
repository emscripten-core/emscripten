@echo off
:: Entry point for running python scripts on windows systems.
:: To modify this file, edit `tools/run_python.bat` and then run
:: `tools/create_entry_points.py`

@IF "%PYTHON%"=="" (
  @SET PYTHON=%EMSDK_PYTHON%
)

@IF "%PYTHON%"=="" (
  @SET PYTHON=python
)

%PYTHON% "%~dp0\%~n0.py" %*
