.. _CodeGuidelinesAndLimitations:

=========================================================
Portability Guidelines (under-construction)
=========================================================

This article describes the types of code that are more difficult to compile, code which compiles but may run more slowly, and issues and limitations with specific APIs. Developers will find it useful for evaluating the effort to port and re-write code.

Code that cannot be compiled
=============================

The following issues are fundamental difficulties. We can get around them by emulation in theory, but it would be very slow.

-  Code that is multithreaded and uses shared state. JavaScript has threads - web workers - but they cannot share state, instead they pass messages.
-  Nonportable code that relies on endianness is problematic. It can work with :ref:`typed-arrays-mode-2`, but it will not be portable!
-  Nonportable code that relies on x86 alignment behavior. X86 allows unaligned reads and writes (so you can read a 16-bit value from a non-even address, i.e., which is only 8-bit aligned address, for example), but other archs do not: ARM will raise ``SIGILL``, and when Emscripten generates JavaScript you will get undefined behavior in this case. (If you build your code with ``SAFE_HEAP=1`` then you will get a clear runtime exception, see :ref:`Debugging`. 

	.. note:: The ``UNALIGNED_MEMORY`` code generation mode can support unaligned code like this, but it is very slow.
	
-  Nonportable code that uses low-level features of the native environment, like native stack manipulation (e.g. in conjunction with setjmp/longjmp. We support normal setjmp/longjmp, but not with stack replacing etc.)
-  Nonportable code that scans registers or the stack, for example to do conservative garbage collection. A variable in a register or on the stack may be held in a JavaScript local variable, which cannot be scanned. (However, you can do conservative scanning when there is no other code on the stack, e.g. from an iteration of the main event loop.)


Code that compiles but might run slowly
=======================================

None of the issues here is a showstopper, but they might be slower than you expect. You probably don't need to worry about this stuff, but it might be useful to know about it.

-  64-bit ints. Bitwise operations on these are reasonable, but math (+, -, \*, /) is emulated, very slowly. JavaScript does not have native 64-bit ints so this is unavoidable.
-  32-bit multiplication that needs all 64 bits of precision must be emulated like 64-bit integer math (JavaScript doubles work fine up to ~52 bits). By default precise 32-bit multiplication is off as it makes a lot of common 32-bit code very slow, but if you need it you can enable it with ``-s PRECISE_I32_MUL=1``, see `src/settings.js <https://github.com/kripken/emscripten/blob/master/src/settings.js#L121>`_ for more.
-  32-bit floats will end up as 64-bit doubles in JavaScript engines. That means they might be a little slower than expected, depending on your CPU.
-  ``memcpy``/``memset`` works, but JavaScript engines don't optimize it very well. Native code will use SIMD on this, JavaScript engines will not - yet.
-  Exceptions and ``longjmp``. In JavaScript such code generally makes the JavaScript engine turn off various optimizations. For that reason exceptions are turned off by default in ``-O1`` and above (to re-enable them, run *emcc* with ``-s DISABLE_EXCEPTION_CATCHING=0``). ``setjmp`` also prevents relooping around it.

Code that should be avoided when easy to do
===========================================

Avoiding these kinds of code might be nice to do, to prevent warnings and to enable additional optimizations. But unless it is very easy for you to avoid them, just ignore this section.

-  Unions. These will trigger ``SAFE_HEAP`` warnings (like *Valgrind*), and make the ``QUANTUM_SIZE=1`` :ref:`optimization <code-generation-modes-quantum-size>` impossible.
-  Bitfields. Also ``SAFE_HEAP`` warnings.
-  Reusing memory, that is, using the same allocated block of memory once for one kind of object, later for another. This prevents the ``QUANTUM_SIZE=1`` optimization.

Libraries
=========

Networking
----------

-  Emscripten supports libc networking functions, but since JavaScript networking is asynchronous, you must limit yourself to asynchronous (nonblocking) operations.


