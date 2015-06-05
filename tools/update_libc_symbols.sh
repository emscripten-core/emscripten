~/Dev/fastcomp/build/bin/llvm-nm ~/.emscripten_cache/libc.bc > a
#cp system/lib/libc.symbols a
grep -v " d " a > b
grep -v " t " b > c
mv c system/lib/libc.symbols

