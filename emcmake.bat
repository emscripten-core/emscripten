:: Entry point for running python scripts on windows systems.
:: To modify this file, edit `tools/run_python.bat` and then run
:: `tools/create_entry_points.py`

@setlocal
@set EM_PY=%EMSDK_PYTHON%
@if "%EM_PY%"=="" (
  set EM_PY=python
)

@set EM_LAUNCHER=1
@"%EM_PY%" "%~dp0\%~n0.py" %*
