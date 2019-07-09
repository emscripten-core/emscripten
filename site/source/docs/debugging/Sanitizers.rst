.. _Sanitizers:

=========================
Debugging with Sanitizers
=========================

.. _sanitizer_ubsan:

Undefined Behaviour Sanitizer
=============================

Clang's `undefined behavior sanitizer`__ (UBSan) is now available for use with
Emscripten. This makes it much easier to catch bugs in your code.

__ https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html

To use UBSan, simply pass ``-fsanitize=undefined`` to ``emcc`` or ``em++``.

Catching Null Dereference
-------------------------

By default, with Emscripten, deferencing a null pointer does not immediately
cause a segmentation fault, unlike traditional platforms. Instead, it checks
a magic cookie stored at address 0 at the end of the program execution.

This only detects null pointer writes, and not reads, and it's rather difficult
to find where the null pointer write occurred.

Emscripten provides a ``SAFE_HEAP`` mode, which can be activated by running
``emcc`` with ``-s SAFE_HEAP=1``. This will catch both null pointer reads and
writes, and causes an exception, but it is slower, and does not tell you the
exact line numbers unless you compile with ``-g``.

UBSan will tell you exactly where the null deference happened, and works for
both reads and writes, with much less performance penalty than ``SAFE_HEAP``.

Consider the following program, ``null-assign.c``:

.. code-block:: c

  int main(void) {
      int *a = 0;
      *a = 0;
  }

Without UBSan or ``SAFE_HEAP``, you get an error when the program exits:

.. code-block:: console

  $ emcc null-assign.c
  $ node a.out.js
  Runtime error: The application has corrupted its heap memory area (address zero)!

With UBSan, you get the exact line number where this happened:

.. code-block:: console

  $ emcc -fsanitize=undefined null-assign.c
  $ node a.out.js
  null-assign.c:3:5: runtime error: store to null pointer of type 'int'
  Runtime error: The application has corrupted its heap memory area (address zero)!

Consider the following program, ``null-read.c``:

.. code-block:: c

  int main(void) {
      int *a = 0, b;
      b = *a;
  }

Without UBSan or ``SAFE_HEAP``, there is no feedback:

.. code-block:: console

  $ emcc null-read.c
  $ node a.out.js
  $

With UBSan, you get the exact line number where this happened:

.. code-block:: console

  $ emcc -fsanitize=undefined null-assign.c
  $ node a.out.js
  null-read.c:3:9: runtime error: load of null pointer of type 'int'

Minimal Runtime
---------------

UBSan's runtime is non-trivial, and its use can unnecessarily increase the
attack surface. For this reason, there is a minimal UBSan runtime that is
designed for production uses.

The minimal runtime is supported by Emscripten. To use it, pass the flag
``-fsanitize-minimal-runtime`` in addition to your ``-fsanitize`` flag.

.. code-block:: console

  $ emcc -fsanitize=null -fsanitize-minimal-runtime null-read.c
  $ node a.out.js
  ubsan: type-mismatch
  $ emcc -fsanitize=null -fsanitize-minimal-runtime null-assign.c
  $ node a.out.js
  ubsan: type-mismatch
  Runtime error: The application has corrupted its heap memory area (address zero)!
