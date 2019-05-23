.. _Sanitizers:

=========================
Debugging with Sanitizers
=========================

Undefined Behaviour Sanitizer
=============================

Clang's `undefined behavior sanitizer`__ (UBSan) is now available for use with
Emscripten. This makes it much easier to catch bugs in your code.

__ https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html

To use UBSan, simply pass ``-fsanitize=undefined`` to ``emcc`` or ``em++``.

Catching Null Dereference
-------------------------

With Emscripten, deferencing a null pointer does not immediately cause a
segmentation fault, unlike traditional platforms. Instead, Emscripten checks
at the end of the program execution. This makes is difficult to find where the
null dereference occurred.

UBSan will tell you exactly where the null deference happened.

Consider the following program, ``null-assign.c``:

.. code-block:: c

  int main(void) {
      int *a = 0;
      *a = 0;
  }

Without UBSan, you get an error when the program exits:

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
