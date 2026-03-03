:: Build pylauncher.exe using MSVC.
::
:: This links against ucrtbase.dll (via ucrt.lib) which ships as part of the OS
:: since Windows 10 (2015) and via Windows Update for Vista/7/8/8.1.
::
:: /O1 : Favor small code (optimization for size)
:: /GS- : Disable buffer security checks (requires vc runtime and not necessary for our tiny command line wrapper)
:: /NODEFAULTLIB : Do not link the default libraries
:: /ENTRY:main : Use main() as entry point directly (no CRT startup)
:: /Brepro : Deterministic (reproducible) output
:: ucrt.lib : Link only against Universal CRT (no vcruntime dependency)

cl pylauncher.c /Fe:pylauncher.exe /O1 /GS- /link /NODEFAULTLIB /ENTRY:main /MACHINE:X64 /Brepro ucrt.lib kernel32.lib shlwapi.lib
