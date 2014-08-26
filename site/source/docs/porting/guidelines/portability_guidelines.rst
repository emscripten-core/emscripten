.. _CodeGuidelinesAndLimitations:

=========================================================
Portability Guidelines (under-construction)
=========================================================

Emscripten can be used to compile almost any *portable* C++/C code to JavaScript.  

This section explains what types of code are non-portable (or more difficult to port), and what code can be compiled but will run slowly. Developers will find it useful for evaluating the effort to port and re-write code.

Non-portable code (code that cannot be compiled)
================================================

The following types of code are fundamentally non-portable, and would need to be re-written to work with Emscripten. While in theory it might be possible to workaround these issues using emulation, it would be very slow.

-  Code that is multi-threaded and uses shared state. JavaScript has threads (web workers), but they cannot share state — instead they pass messages.
-  Code that relies on endianness is problematic. It can work with :ref:`typed-arrays-mode-2`, but it will not be portable!
-  Code that relies on x86 alignment behavior. X86 allows unaligned reads and writes (so for example you can read a 16-bit value from a non-even address), but other architectures do not (ARM will raise ``SIGILL``). For Emscripten-generated JavaScript the behavior is undefined. If you build your code with ``SAFE_HEAP=1`` then you will get a clear runtime exception, see :ref:`Debugging`. 

	.. note:: The `UNALIGNED_MEMORY <https://github.com/kripken/emscripten/blob/master/src/settings.js#L99>`_ code generation mode can support unaligned code like this, but it is very slow.
	
-  Code that uses low-level features of the native environment. For example, native stack manipulation in conjunction with ``setjmp``/``longjmp`` (we support normal ``setjmp``/``longjmp``, but not with stack replacing etc.)
-  Code that scans registers or the stack. This won't work because a variable in a register or on the stack may be held in a JavaScript local variable (which cannot be scanned).
	
	.. note:: Code of this type might be used for conservative garbage collection. You can do conservative scanning when there is no other code on the stack, e.g. from an iteration of the main event loop.


Code that compiles but might run slowly
=======================================

.. note:: Understanding these issues can be helpful when optimising code.

The following types of code will compile, but may not run as fast as expected:

-  64-bit ``int`` variables. Mathematical operations (+, -, \*, /) are slow because they are emulated (bitwise operations are reasonably fast). JavaScript does not have a native 64-bit ``int`` type so this is unavoidable.
-  32-bit multiplication that needs all 64 bits of precision is very slow because it is emulated:

	- The JavaScript ``double`` type works fine up to ~52 bits.
	- Precise 32-bit multiplication is disabled by default because it makes a lot of common 32-bit code very slow. It can be enabled by calling :ref:`emcc <emccdoc>` with ``-s PRECISE_I32_MUL=1`` (see `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js#L121>`_ for more information).
-  32-bit ``float`` variables will become 64-bit ``double`` variables in JavaScript engines. That means they might be a little slower than expected, depending on your CPU.
-  ``memcpy``/``memset`` works, but JavaScript engines don't optimize it very well. 

	.. note:: Native code will use SIMD on this. JavaScript engines will not - yet.
	
-  Exceptions and ``longjmp``. In JavaScript such code generally makes the JavaScript engine turn off various optimizations. For that reason exceptions are turned off by default in ``-O1`` and above. To re-enable them, run *emcc* with ``-s DISABLE_EXCEPTION_CATCHING=0`` (see `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js#L279>`_). ``setjmp`` also prevents relooping around it.

Code that should be avoided when easy to do so
==============================================

These types of code can trigger warnings and prevent certain optimizations. The code should be avoided, but only if it is very easy to do so:

-  Unions. These will trigger ``SAFE_HEAP`` warnings (like *Valgrind*), and make the ``QUANTUM_SIZE=1`` :ref:`optimization <code-generation-modes-quantum-size>` impossible.
-  Bitfields. These will trigger ``SAFE_HEAP`` warnings.
-  Reusing memory. Using the same allocated block of memory once for one kind of object and later for another prevents the ``QUANTUM_SIZE=1`` optimization.

