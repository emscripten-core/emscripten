:: /MT :  Statically link to the C runtime library for max portability
:: /O1 : Favor small code (optimization for size)

cl pylauncher.c /Fe:pylauncher.exe /MT /O1
