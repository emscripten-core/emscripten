Building Python with Emscripten
===============================

These directions should work for Python 2.7.x (last tested with 2.7.5).

First, uncompress Python into two separate directories, one for native
and one for JavaScript.

In the JavaScript directory, do:

````
    EMCONFIGURE_JS=1 emconfigure ./configure --without-threads --without-pymalloc --enable-shared --disable-ipv6
````

If you are on Mac OS X, you will also want ``disable-toolbox-glue``.
If you are on an older version of Python (such as 2.7.2), you may
not need the ``--disable-ipv6`` option.

If you are on Python 2.7.4 or later, you will need to edit the
``Makefile`` generated and remove the ``MULTIARCH=`` line(s).
You will also need to edit ``pyconfig.h`` and remove defines
for ``DOUBLE_IS_BIG_ENDIAN_IEEE754``, ``DOUBLE_IS_ARM_MIXED_ENDIAN_IEEE754``,
and ``HAVE_GCC_ASM_FOR_X87``.

On Python 2.7.2, you will need to edit ``pyconfig.h`` and remove
``HAVE_GCC_ASM_FOR_X87``, ``HAVE_SIG*`` except for ``SIGNAL_H``
and *add* ``#define PY_NO_SHORT_FLOAT_REPR``.

Now, you can run ``make``. It will fail trying to run ``pgen``.

At this point, go to your native directory and run:

````
./configure --without-threads --without-pymalloc --enable-shared --disable-ipv6
````

Now, run ``make`` in the native directory and then copy the generated ``Parser/pgen``
to your JavaScript directory. Back in your JavaScript directory, be sure to flag
the ``pgen`` executable as executable:

````
chmod +x Parser/pgen
````

Now, run ``make`` again.

You will get an error about trying to run ``python`` or ``python.exe``. This
can be ignored.

Now, you can link the bitcode file that you need:

````
llvm-link libpython2.7.so Modules/python.o -o python.bc
````

If you are on Mac OS X, you will want to look for ``libpython2.7.dylib``
instead of ``libpython2.7.so``.

Thanks to rasjidw and everyone else who has helped with this!
