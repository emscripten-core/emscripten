:: /MT :  Statically link to the C runtime library for max portability
:: /O1 : Favor small code (optimization for size)
:: /Brepro : Use deterministic (reproducible) output without timestamps

cl pylauncher.c /Fe:pylauncher.exe /MT /O1 /link /Brepro
