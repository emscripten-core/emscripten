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

