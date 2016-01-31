.. _code-portability-guidelines:

======================
Portability Guidelines
======================

Emscripten can be used to compile almost any *portable* C++/C code to JavaScript.  

This section explains what types of code are non-portable (or more difficult to port), and what code can be compiled but will run slowly. Developers can use this information for evaluating the effort to port and re-write code.

Code that cannot be compiled
============================

The following types of code would need to be re-written in order to work with Emscripten. (While in theory it might be possible for Emscripten to work around these issues using emulation, it would be very slow.)

-  Code that is multi-threaded and uses shared state. JavaScript has threads (web workers), but they cannot share state — instead they pass messages. 

	.. note:: Should the JavaScript standards bodies add shared state to web workers, multithreaded code would become possible to support.
	
-  Code relying on a big-endian architecture. Emscripten-compiled code currently requires a little-endian host to run on, which accounts for 99% of machines connected to the internet. This is because JavaScript typed arrays (used for views on memory) obey the host byte ordering and LLVM needs to know which endianness to target.
-  Code that relies on x86 alignment behavior. x86 allows unaligned reads and writes (so for example you can read a 16-bit value from a non-even address), but other architectures do not (ARM will raise ``SIGILL``). For Emscripten-generated JavaScript the behavior is undefined. If you build your code with ``SAFE_HEAP=1`` then you will get a clear runtime exception, see :ref:`Debugging`.
-  Code that uses low-level features of the native environment, for example native stack manipulation in conjunction with ``setjmp``/``longjmp`` (we support proper ``setjmp``/``longjmp``, i.e., jumping down the stack, but not jumping up to an unwound stack, which is undefined behavior).
-  Code that scans registers or the stack. This won't work because a variable in a register or on the stack may be held in a JavaScript local variable (which cannot be scanned).
	
	.. note:: Code of this type might be used for conservative garbage collection. You can do conservative scanning when there is no other code on the stack, e.g. from an iteration of the main event loop.

-  Code with architecture-specific inline assembly (like an ``asm()`` containing x86 code) is not portable. That code would need to be replaced with portable C or C++. Sometimes a codebase will have both portable code and optional inline assembly as an optimization, so you might find an option to disable the inline assembly.


Code that compiles but might run slowly
=======================================

.. note:: Understanding these issues can be helpful when optimising code.

The following types of code will compile, but may not run as fast as expected:

-  64-bit ``int`` variables. Mathematical operations (+, -, \*, /) are slow because they are emulated (bitwise operations are reasonably fast). JavaScript does not have a native 64-bit ``int`` type so this is unavoidable.
	
-  C++ Exceptions. In JavaScript such code generally makes the JavaScript engine turn off various optimizations. For that reason exceptions are turned off by default in ``-O1`` and above. To re-enable them, run *emcc* with ``-s DISABLE_EXCEPTION_CATCHING=0`` (see `src/settings.js <https://github.com/kripken/emscripten/blob/1.29.12/src/settings.js#L298>`_). 

- ``setjmp`` also prevents :term:`relooping` around it, forcing us to emulate control flow using a less efficient approach.

