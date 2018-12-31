Building Python with Emscripten
===============================

These directions should work for Python 2.7.x (last tested with 2.7.8)
and are based on https://github.com/aidanhs/empython

First, uncompress Python into two separate directories, one for native
and one for JavaScript.

In the JavaScript directory, do:

````
BASECFLAGS=-m32 LDFLAGS=-m32 emconfigure ./configure --without-threads --without-pymalloc --disable-shared --without-signal-module --disable-ipv6
````

If you are on Mac OS X, you will also want ``disable-toolbox-glue``.
If you are on an older version of Python (such as 2.7.2), you may
not need the ``--disable-ipv6`` option.

If you are on Python 2.7.4 or later, you will need to edit the
``Makefile`` generated and remove the ``MULTIARCH=`` line(s).
You will also need to edit ``pyconfig.h`` and remove the define
for ``HAVE_GCC_ASM_FOR_X87``.

On Python 2.7.2, you will need to edit ``pyconfig.h`` and remove
``HAVE_GCC_ASM_FOR_X87``, ``HAVE_SIG*`` except for ``SIGNAL_H``
and *add* ``#define PY_NO_SHORT_FLOAT_REPR``.

Now, you can run ``make``. It may fail trying to run ``pgen``.

If so, go to your native directory and run:

````
./configure && make Parser/pgen
````

Now, copy the generated ``Parser/pgen``
to your JavaScript directory. Back in your JavaScript directory, be sure to flag
the ``pgen`` executable as executable:

````
chmod +x Parser/pgen
````

Now, run ``make`` again.

You will get an error about trying to run ``python`` or ``python.exe``. This
can be ignored (unless you want to build C modules, in which case you will
need to copy a native build of Python and edit Modules/Setup appropriately).

The bitcode file you need has already been linked, so just rename it

````
mv python python.bc
````
