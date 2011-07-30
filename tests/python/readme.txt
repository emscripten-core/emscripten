This is Python 2.7.2, compiled to .ll as follows:

  Untar Python.
  In the Python dir, create a ./bin directory
  Copy ccproxy.py to there
  CC=./ccproxy.py ../configure --without-threads --without-pymalloc
  EDIT pyconfig.h (in ./bin), remove
    HAVE_GCC_ASM_FOR_X87
    HAVE_SIG* except SIGNAL_H
  and *add*
    #define PY_NO_SHORT_FLOAT_REPR
  make
  ...it will fail, but can continue manually
  cd pylibs
  ar x ../libpython2.7.a
  cp ../Modules/python.o .
  LLVM_DIR/llvm-link -o=python.bc *.o
  LLVM_DIR/llvm-dis -show-annotations python.bc

Thanks go to rasjidw for helping with this!

