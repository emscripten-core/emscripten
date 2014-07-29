.. _Tutorial:

======================
Tutorial (wiki-import)
======================
.. note:: This article was migrated from the wiki (Fri, 25 Jul 2014 04:21) and is now the "master copy" (the version in the wiki will be deleted). It may not be a perfect rendering of the original but we hope to fix that soon!

Emscripten Tutorial
===================

Emscripten is an open source LLVM to JavaScript compiler. With it, you
can compile C and C++ code into JavaScript and run it on the web. And
it's easy! This tutorial will show you how to use it.

Requirements
------------

To use Emscripten and complete this tutorial, first `get
Emscripten <https://github.com/kripken/emscripten/wiki/Emscripten-SDK>`__!

Before continuing, it's a good idea to make sure the requirements work.

First, change directory to where Emscripten is located, the rest of this
tutorial will assume that that is where you are running commands (or,
add a path to another location if you prefer). Then try

::

    clang tests/hello_world.cpp
    ./a.out

(Add the path to clang if it isn't installed systemwide.) That uses
Clang and LLVM to compile a "hello world" app and run it. The second
command there should print "hello, world!". Then, test Node.js with

OS X Note: Since 10.9 Apple has decided that the standard C++ libraries
are not part of the system, but part of XCode. If you get the following
error ``fatal error: 'stdio.h' file not found`` then run
``xcode-select --install`` to install the command line tools before
trying again.

::

    node src/hello_world.js

which should also print out "hello, world!". (As before, add the path to
node if it isn't installed systemwide.)

Setting up Emscripten
---------------------

If you haven't run Emscripten before, run it now with

::

    ./emcc

If you are on Windows, you will call

::

    emcc

instead. If you are having trouble with python versions, you can also
explicitly invoke

::

    python emcc

especially if ``python2`` is not defined in your system (``python2``
allows python 2 and 3 to be installed together on one system, which is
increasingly common; as an alternative to ``python emcc``, you can also
add a symlink to ``python`` from ``python2``). In that case you should
also update PYTHON in ``~/.emscripten``, see below about that file.

"emcc" is the "Emscripten compiler frontend", an easy way to use
Emscripten basically as a drop-in replacement for a standard compiler
like gcc.

The first time you run emcc (or any other of the Emscripten tools), it
will create a settings file at ``~/.emscripten`` (``~`` is your user's
home directory) and exit. You should edit that file now, changing the
directory locations of LLVM and Node to the right places in your setup
(specifically, edit ``LLVM_ROOT`` and ``NODE_JS``). If those paths are
not right, Emscripten will not find LLVM, Clang or Node.js and it will
fail. Look at the comments in that file that explain what the settings
are and which you need to change.

After setting those paths, run ``./emcc`` again. It should do some
sanity checks which test the specified paths in ``~/.emscripten``. If
they don't all pass, you might have a typo somewhere. When everything is
set up properly, running ``./emcc`` should tell you
``emcc: no input files`` (since we didn't specify any input files), and
you can proceed to the next section in this tutorial.

Running Emscripten
------------------

You can now compile your first file! First, let's build the same "hello
world" C++ file from before, but this time using Emscripten:

::

    ./emcc tests/hello_world.cpp

There should now be an ``a.out.js`` file in the current directory. Run
it with

::

    node a.out.js

and it should print "hello, world!" as expected.

-  If an error occurs when running emcc, try to run it with ``-v``,
   which will print out information that can help pinpoint the problem.

Generating HTML
~~~~~~~~~~~~~~~

Emscripten can also generate HTML with embedded JavaScript. Try this
command:

::

    ./emcc tests/hello_world_sdl.cpp -o hello.html

By specifying that the output is an HTML file, you have told Emscripten
to generate a complete HTML page. In this case, the source code uses the
SDL API to show a colored cube. Open the web page in your web browser to
see it (it should work in any browser that supports the Canvas element.)

Using Files
~~~~~~~~~~~

Your C/C++ code can access files using the normal libc API (stdio:
fopen, etc.). Try this command:

::

    ./emcc tests/hello_world_file.cpp -o hello.html --preload-file tests/hello_world_file.txt

Open ``hello.html`` in a web browser and you will see the data from a
file being written out. (Note: Chrome is unable to do ``file://`` XHRs,
so for ``hello.html`` to work in that browser you need a webserver, for
example ``python -m SimpleHTTPServer 8888`` and then open
``localhost:8888/hello.html``.) Open ``tests/hello_world_file.cpp`` to
see the C++ source code, and ``tests/hello_world_file.txt`` to see the
data. The ``--preload-file`` option will automatically preload the file
before running the compiled code. This is useful because loading data
from the network cannot be done synchronously in browsers outside Web
Workers. Almost all C/C++ code is synchronous, so preloading is the
simplest solution.

For an example of files with SDL, change to the ``tests/`` directory and
run

::

    ../emcc hello_image_sdl.c --preload-file screenshot.jpg -o a.html

Then browse to a.html. Note that we must run the command from tests/,
since file preloading will generate files in the virtual filesystem that
correspond to the current filesystem as it is seen at compile time. When
we run the command in tests/, then the file is accessible as
``screenshot.jpg`` and not ``tests/screenshot.jpg``, and the former is
what the code expects.

See also: :ref:`Synchronous-virtual-XHR-backed-file-system-usage`.

Optimizing Code
---------------

Emscripten will by default generate unoptimized code, just like gcc
does. You can generate slightly-optimized code with ``-O1``, for example

::

    ./emcc -O1 tests/hello_world.cpp

The "hello world" code here doesn't really need to be optimized, so you
won't see a difference in speed when running it. But, you can look at
the generated code to see the differences: ``-O1`` applies several minor
optimizations to the code (simple ones that don't increase compilation
time), and removes some runtime assertions. For example, ``printf`` will
have been replaced by ``puts`` in the generated code.

Further optimizations are done in ``-O2``,

::

    ./emcc -O2 tests/hello_world.cpp

If you inspect the generated code now, you will see it looks very
different.


.. _running-emscripten-tests:

Running the Emscripten Test Suite and Benchmarks
------------------------------------------------

Emscripten has an extensive test suite. You can run it with

::

    python tests/runner.py

This will take a long time, perhaps several hours - there are many many
tests! You can run an individual test as follows:

::

    python tests/runner.py test_hello_world

If you want to view the generated code from that individual test, do
``EMCC_DEBUG=1 python tests/runner.py test_hello_world``, and then you
can look inside the temp directory (``TEMP_DIR/emscripten_temp``, where
``TEMP_DIR`` is defined in ``~/.emscripten`` - by default it is
``/tmp``). Note that you can use ``EMCC_DEBUG`` with emcc in general,
not just with the test runner - it tells emcc to save the internal code
generation stages (much like ``emcc -v``).

A test suite specific feature is ``EM_SAVE_DIR=1`` in the environment,
which will save the temp dir the test runner users, in the same place as
mentioned in the previous paragraph. This is useful if the test you are
running creates some temp files.

Note that Node.js cannot run 100% of the tests in the suite; if you care
about running them all, you should get the SpiderMonkey shell (a recent
trunk version).

You can run the Emscripten benchmarks using

::

    python tests/runner.py benchmark

This will compile a sequence of benchmarks and run them several times,
reporting averaged statistics including a comparison to how fast the
same code runs when compiled to a native executable.

Under the Hood
--------------

The goal in this tutorial is to show you how to use emcc to compile code
to JavaScript. The commands are very simple, and normally you don't need
to understand what goes on underneath. However, if you're curious or you
want to do something more advanced with Emscripten, then understanding
more about how it works can be useful.

Cross-Compiling
~~~~~~~~~~~~~~~

The main 'under the hood' topic to be aware of is that **emcc is a
cross-compiler**: You are on a 'normal' OS, running native code, but
using emcc you are building for a different environment, JavaScript.
Other examples of cross-compiling are building for an ARM phone on an
x86 desktop, etc. When cross-compiling, the thing to keep in mind is
that you need to build with settings for the target platform, not the
one you are currently on. For that reason, Emscripten (and other
cross-compilers) ship with a complete build environment, including
system headers and so forth. When you run emcc, it does **not** use your
/usr/include directory, instead it uses the system headers bundled with
Emscripten (in system/include). One thing to be aware of is if you build
a project that has hardcoded includes, for example
``-I/usr/include/something``: Using system headers that way is dangerous
when you are cross-compiling, since the headers are meant for your local
system, not for the platform you are actually building for.

Emscripten Options
~~~~~~~~~~~~~~~~~~

The Emscripten compiler (the core code called by emcc that translates
LLVM assembly to JavaScript) has various options, which sometimes are
useful to modify. To see the options look in ``src/settings.js``, they
appear there with descriptions of what they do in comments. To modify a
setting, use the ``-s`` option to emcc, for example

::

    emcc source.cpp -s TOTAL_STACK=10000000

This invocation of emcc will generate JavaScript that sets aside a lot
of space for the stack.

General Tips and Next Steps
---------------------------

After finishing this tutorial, here are some general tips for using
Emscripten:

-  There is a lot of useful information on this wiki. In particular, you
   might be interested in the following pages:
-  
-  
-  
-  
-  
-  If the documentation is lacking for something, use the test suite.
   Emscripten has an extensive test suite, and everything in it works
   perfectly on our test machines. For example, if you want to better
   understand how the ``--pre-js`` option to emcc works, search for
   ``--pre-js`` in the test suite (``tests/``, and usually the result
   will be in ``tests/runner.py``).
-  To learn how to use emscripten in advanced ways, read
   ``src/settings.js`` which describes the compiler options, and
   ``system/include/emscripten/emscripten.h`` which describes
   JS-specific C APIs that your C/C++ programs can use when compiled
   with emscripten.
-  Use the links on the main wiki page to the Emscripten IRC channel and
   mailing list. When in doubt, get in touch!

