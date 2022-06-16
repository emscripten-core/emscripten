.. Exceptions support:

==============================
C++ exceptions support
==============================

By default, exception catching is disabled in Emscripten.

For example, if you compile the following program:

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

The first ``throw`` will abort the program and you'll see something like this in the output:

.. code-block:: text

  throw...  
  exception thrown: 5246024 - Exception catching is disabled, this exception cannot be caught. Compile with -sNO_DISABLE_EXCEPTION_CATCHING or -sEXCEPTION_CATCHING_ALLOWED=[..] to catch.

If you want to opt-in, you have two following options.

JavaScript-based exception support
##################################

First, you can enable exceptions via Emscripten's JavaScript-based support.

To enable it, pass ``-fexceptions`` at both compile time and link time.

When you rebuild the example above with this flag, the output will change to:

.. code-block:: text

  throw...
  catch!

Note that this option has relatively high overhead, but it will work on all JavaScript
engines with WebAssembly support. You can reduce the overhead by specifying a
list of allowed functions in which exceptions are enabled, see the
``EXCEPTION_CATCHING_ALLOWED`` setting.

WebAssembly exception handling proposal
#######################################

Alternatively, you can opt-in to the `native WebAssembly exception handling
<https://github.com/WebAssembly/exception-handling/blob/master/proposals/exception-handling/Exceptions.md>`_
proposal.

To enable it, pass ``-fwasm-exceptions`` at both compile time and link time.

Rebuilding the example with this flag will result in the same output as with
``-fexceptions`` above:

.. code-block:: text

  throw...
  catch!

This option leverages a new feature that brings built-in instructions for
throwing and catching exceptions to WebAssembly.

As a result, it can reduce code size and performance overhead compared
to the JavaScript-based implementation. However, it's still brand-new
and `not yet supported by default in most engines <https://webassembly.org/roadmap/>`_.
