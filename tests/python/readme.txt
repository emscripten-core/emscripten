Updates to Python 2.7.4 and emscripten of Arp 29 2013 with le32

Go to js dir, run EMCONFIGURE_JS=1 ~/Dev/emscripten/emconfigure ./configure --without-threads --without-pymalloc --enable-shared --disable-ipv6
clean out MULTIARCH= in Makefile
make, until error on pgen
Go to native, run ./configure --without-threads --without-pymalloc --enable-shared --disable-ipv6
cp Parser/pgen ../JS_DIR/Parser
return to JS
chmod +x Parser/pgen
remove #defines of  DOUBLE_IS_BIG_ENDIAN_IEEE754 and DOUBLE_IS_ARM_MIXED_ENDIAN_IEEE754 and HAVE_GCC_ASM_FOR_X87 in pyconfig.h
make
link libpython2.7.so with Modules/python.o to get the bitcode file you want

=========================


This is Python 2.7.2, compiled to .bc as follows:

Uncompress Python into two separate directories, one for native and one for JS.

In the JS one, do
  emconfigure ./configure --without-threads --without-pymalloc --enable-shared
  EDIT pyconfig.h, remove
    HAVE_GCC_ASM_FOR_X87
    HAVE_SIG* except SIGNAL_H
  and *add*
    #define PY_NO_SHORT_FLOAT_REPR
  make
It will fail on lack of permissions to run Parser/pgen.

Go to the native one, do
  ./configure --without-threads --without-pymalloc
  make
  (Note: you don't need to let it complete, just enough for Parse/pgen is sufficient.)
  cp Parser/pgen ../YOUR_JS_DIR/Parser/

Return to the JS one, do
  chmod +x Parser/pgen
  make
You will get an error on lack of permissions to run ./python. Ignore that, and do
  llvm-link libpython2.7.so python -o python.bc

That's it!

Thanks to rasjidw for helping with this!

