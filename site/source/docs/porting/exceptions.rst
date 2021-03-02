.. Exceptions support:

==============================
C++ exceptions support
==============================

C++ exceptions support can be enabled in emscripten using ``-fexceptions``.
Note that unlike on most platforms, C++ exceptions are disabled by default due
to their current overhead, see later.

Passing ``-fexceptions`` at compile time will pass the relevant flags to
clang, and at link time it also links in system libraries with exceptions
support.

Example
#######

For example, build the following program with ``emcc -fexceptions``:

.. code-block:: cpp

    #include <stdio.h>

    int main() {
      try {
        puts("throw...");
        throw 1;
        puts("(never reached)");
      } catch(...) {
        puts("catch!");
      }
      return 0;
    }

Running it will print ``throw...`` and then ``catch!``.

Overhead
########

Until wasm gets direct support for exceptions, Emscripten supports them using
JavaScript for throwing and catching. As a result, every time we might need to
catch an exception we call out into JavaScript to a try-catch there. This
increases code size and adds significant overhead.

You can reduce the overhead by specifying a list of allowed functions in
which exceptions are enabled, see the ``EXCEPTION_CATCHING_ALLOWED`` setting.

In the future direct support for exceptions in wasm will avoid that overhead.
Usage will be as mentioned earlier, but with ``-fwasm-exceptions`` instead of
``-fexceptions``.
