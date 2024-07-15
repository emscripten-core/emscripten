.. _console-h:

=========
console.h
=========

Functions
---------

.. c:function:: void emscripten_console_log(const char *utf8String)

  Prints a string using ``console.log()``.

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_console_warn(const char *utf8String)

  Prints a string using ``console.warn()``.

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_console_error(const char *utf8String)

  Prints a string using ``console.error()``.

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_out(const char *utf8String)

  Prints a string using the `out()` JS function, which by default will write to
  the console (or stdout), but can be overridden by via the ``print`` attribute
  on the ``Module`` object.

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_err(const char *utf8String)

  Prints a string using the `err()` JS function, which by default will write to
  the console (or stderr), but can be overridden by via the ``printErr``
  attribute on the ``Module`` object.

  :param utf8String: A string encoded as UTF-8.


.. c:function:: void emscripten_dbg(const char *utf8String)

  Prints the string using the `dbg()` JS function, which by will write to the
  console (or stdout).  Just like the `dbg()` JS function this symbol is
  only available in debug builds (i.e. when linking with `-sASSERTIONS` or
  equivelently `-O0`).

  :param utf8String: A string encoded as UTF-8.
