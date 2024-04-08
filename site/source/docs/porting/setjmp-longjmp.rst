.. _setjmp-longjmp:

========================
C setjmp-longjmp Support
========================

setjmp-longjmp support is enabled by default in Emscripten. This is controlled
by the ``SUPPORT_LONGJMP`` setting, which can take these values:

* ``emscripten``: JavaScript-based support
* ``wasm``: WebAssembly exception handling-based support
* 0: No support
* 1: Default support, depending on the exception mode. ``wasm`` if ``-fwasm-exception`` is used, ``emscripten`` otherwise.

If :ref:`native Wasm exceptions <webassembly-exception-handling-based-support>`
are used, ``SUPPORT_LONGJMP`` defaults to ``wasm``, and if :ref:`JavaScript-based
exceptions <javascript-based-exception-support>` are used or no exception
support is used, it defaults to ``emscripten``.

``setjmp`` saves information about the calling environment into a buffer, and
``longjmp`` transfers the control back to the point where ``setjmp`` was called
by using the buffer. ``longjmp``'s call stack should contain the function from
which ``setjmp`` was called.

Emscripten's support has a restriction that indirect calls to ``setjmp`` are not
supported. For example, the below does not work:

.. code-block:: c

  jmp_buf env;
  int (*fp)(jmp_buf) = setjmp;
  fp(env); // Doesn't work


JavaScript-based setjmp-longjmp Support
=======================================

In this mode, Emscripten emulates setjmp-longjmp using JavaScript. This option
is set by adding ``-sSUPPORT_LONGJMP=emscripten`` to the command line, but this
is currently enabled by default.

Note that this option can have relatively high overhead in terms of code size,
but it will work on all JavaScript engines with WebAssembly support, even if
they do not yet support the `new WebAssembly exception handling proposal
<https://github.com/WebAssembly/exception-handling/blob/master/proposals/exception-handling/Exceptions.md>`_.


WebAssembly Exception Handling-based setjmp-longjmp Support
===========================================================

Alternatively, you can opt-in to the new support using the `WebAssembly
exception handling
<https://github.com/WebAssembly/exception-handling/blob/master/proposals/exception-handling/Exceptions.md>`_
proposal. To enable it, pass ``-sSUPPORT_LONGJMP=wasm`` at both compile time and
link time.

This option leverages a new feature that brings built-in instructions for
throwing and catching exceptions to WebAssembly. As a result, it can reduce code
size and performance overhead compared to the JavaScript-based implementation.
This option is currently supported in several major web browsers, but `may not
be supported in all WebAssembly engines yet
<https://webassembly.org/roadmap/>`_.


.. _using-exceptions-and-setjmp-longjmp-together:

Using Exceptions and setjmp-longjmp Together
============================================

We also have two kinds of :ref:`exception handling support <exceptions>`:
JavaScript-based support and the new WebAssembly EH-based support. Our
setjmp-longjmp support use the same mechanisms. Because of that, you should use
the same kind of EH and setjmp-longjmp support when using exceptions and
setjmp-longjmp together.

For example, to use the JavaScript-based EH and setjmp-longjmp support together:

.. code-block:: bash

  em++ -fexceptions test.cpp -o test.js

``-sSUPPORT_LONGJMP``, which defaults to ``emscripten`` or ``wasm`` depending on
the exception mode, is enabled by default, so you don't need to pass it
explicitly.

To use the WebAssembly EH and setjmp-longjmp support together:

.. code-block:: bash

  em++ -fwasm-exceptions -sSUPPORT_LONGJMP=wasm test.cpp -o test.js

There is one specific restriction for using WebAssembly EH-based support for
exceptions and setjmp-longjmp at the same time. You cannot call ``setjmp``
within a C++ ``catch`` clause. For example, the following will error out during
compile time:

.. code-block:: cpp

  try {
    ...
  catch (int n) {
    setjmp(buf); // Doesn't work
  }

Calling ``setjmp`` within a ``try`` clause is fine. Calling another user
function that calls ``setjmp`` within a ``catch`` clause is also fine.

.. code-block:: cpp

  try {
    setjmp(buf); // Works
  catch (int n) {
    ...
  }

  try {
    ...
  } catch (int n) {
    function_that_calls_setjmp(); // Works
  }
