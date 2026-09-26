:: Build pylauncher.exe using MSVC.
::
:: This links against ucrtbase.dll (via ucrt.lib) which ships as part of the OS
:: since Windows 10 (2015) and via Windows Update for Vista/7/8/8.1.
::
:: /O1 : Favor small code (optimization for size)
:: /MT : Statically link VC runtime and startup code (no vcruntime140.dll dependency)
:: /Brepro : Deterministic (reproducible) output
:: /NODEFAULTLIB:libucrt.lib ucrt.lib : Dynamically link Universal CRT (ucrtbase.dll)

set OUT=pylauncher.exe
set MACHINE=X64

if /i "%~1"=="arm64" (
  set OUT=pylauncher-arm64.exe
  set MACHINE=ARM64
)

rc /nologo /fo pylauncher.res pylauncher.rc
cl pylauncher.c pylauncher.res /Fe:%OUT% /O1 /MT /Brepro /link /NODEFAULTLIB:libucrt.lib /MACHINE:%MACHINE% ucrt.lib
