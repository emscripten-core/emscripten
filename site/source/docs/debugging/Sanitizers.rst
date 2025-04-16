.. _Sanitizers:

=========================
Debugging with Sanitizers
=========================

.. _sanitizer_ubsan:

Undefined Behaviour Sanitizer
=============================

Clang's `undefined behavior sanitizer`__ (UBSan) is available for use with
Emscripten. This makes it much easier to catch bugs in your code.

__ https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html

To use UBSan, simply pass ``-fsanitize=undefined`` to ``emcc`` or ``em++``. Note
that you need to pass this at both the compile and link stages, as it affects
both codegen and system libraries.

Catching Null Dereference
-------------------------

By default, with Emscripten, dereferencing a null pointer does not immediately
cause a segmentation fault, unlike traditional platforms, as 0 is just a normal
address in a WebAssembly memory. 0 is also a normal location in a
JavaScript Typed Array, which is an issue in the JavaScript alongside the
WebAssembly (runtime support code, JS library methods, ``EM_ASM/EM_JS``, etc.),
and also for the compiled code if you build with ``-sWASM=0``.

In builds with ``ASSERTIONS`` enabled, a magic cookie stored at address 0 is
checked at the end of the program execution. That is, it will notify you if
anything wrote to that location while the program ran. This only detects writes,
not reads, and does not help to find where the bad write actually is.

Consider the following program, ``null-assign.c``:

.. code-block:: c

  int main(void) {
      int *a = 0;
      *a = 0;
  }

Without UBSan you get an error when the program exits:

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

Without UBSan there is no feedback:

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

.. _sanitizer_asan:

Address Sanitizer
=================

Clang's `address sanitizer`__ (ASan) is also available for use with Emscripten.
This makes it much easier to catch buffer overflows, memory leaks, and other
related bugs in your code.

__ https://clang.llvm.org/docs/AddressSanitizer.html

To use ASan, simply pass ``-fsanitize=address`` to ``emcc`` or ``em++``. As
with UBSan, you need to pass this at both the compile and link stages,
as it affects both codegen and system libraries.

You probably need to increase :ref:`INITIAL_MEMORY` to at least 64 MB or set
:ref:`ALLOW_MEMORY_GROWTH` so that ASan has enough memory to start. Otherwise,
you will receive an error message that looks something like:

  Cannot enlarge memory arrays to size 55152640 bytes (OOM). Either (1) compile
  with ``-sINITIAL_MEMORY=X``  with X higher than the current value 50331648, (2)
  compile with ``-sALLOW_MEMORY_GROWTH``  which allows increasing the size at
  runtime, or (3) if you want malloc to return NULL (0) instead of this abort,
  compile with ``-sABORTING_MALLOC=0``

ASan fully supports multi-thread environments. ASan also operates on the JS
support code, that is, if JS tries to read from a memory address that is not
valid, it will be caught, just like if that access happened from Wasm.

Examples
--------

Here are some examples of how AddressSanitizer can be used to help find bugs.

Buffer Overflow
^^^^^^^^^^^^^^^

Consider ``buffer_overflow.c``:

.. code-block:: c

  #include <string.h>

  int main(void) {
    char x[10];
    memset(x, 0, 11);
  }

.. code-block:: console

  $ emcc -gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH buffer_overflow.c
  $ node a.out.js
  =================================================================
  ==42==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x02965e5a at pc 0x000015f0 bp 0x02965a30 sp 0x02965a30
  WRITE of size 11 at 0x02965e5a thread T0
      #0 0x15f0 in __asan_memset+0x15f0 (a.out.wasm+0x15f0)
      #1 0xc46 in __original_main stack_buffer_overflow.c:5:3
      #2 0xcbc in main+0xcbc (a.out.wasm+0xcbc)
      #3 0x800019bc in Object.Module._main a.out.js:6588:32
      #4 0x80001aeb in Object.callMain a.out.js:6891:30
      #5 0x80001b25 in doRun a.out.js:6949:60
      #6 0x80001b33 in run a.out.js:6963:5
      #7 0x80001ad6 in runCaller a.out.js:6870:29

  Address 0x02965e5a is located in stack of thread T0 at offset 26 in frame
      #0 0x11  (a.out.wasm+0x11)

    This frame has 1 object(s):
      [16, 26) 'x' (line 4) <== Memory access at offset 26 overflows this variable
  HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
        (longjmp and C++ exceptions *are* supported)
  SUMMARY: AddressSanitizer: stack-buffer-overflow (a.out.wasm+0x15ef)
  ...

Use After Free
^^^^^^^^^^^^^^

Consider ``use_after_free.cpp``:

.. code-block:: cpp

  int main() {
    int *array = new int[100];
    delete [] array;
    return array[0];
  }

.. code-block:: console

  $ em++ -gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH use_after_free.cpp
  $ node a.out.js
  =================================================================
  ==42==ERROR: AddressSanitizer: heap-use-after-free on address 0x03203e40 at pc 0x00000c1b bp 0x02965e70 sp 0x02965e7c
  READ of size 4 at 0x03203e40 thread T0
      #0 0xc1b in __original_main use_after_free.cpp:4:10
      #1 0xc48 in main+0xc48 (a.out.wasm+0xc48)

  0x03203e40 is located 0 bytes inside of 400-byte region [0x03203e40,0x03203fd0)
  freed by thread T0 here:
      #0 0x5fe8 in operator delete[](void*)+0x5fe8 (a.out.wasm+0x5fe8)
      #1 0xb76 in __original_main use_after_free.cpp:3:3
      #2 0xc48 in main+0xc48 (a.out.wasm+0xc48)
      #3 0x800019b5 in Object.Module._main a.out.js:6581:32
      #4 0x80001ade in Object.callMain a.out.js:6878:30
      #5 0x80001b18 in doRun a.out.js:6936:60
      #6 0x80001b26 in run a.out.js:6950:5
      #7 0x80001ac9 in runCaller a.out.js:6857:29

  previously allocated by thread T0 here:
      #0 0x5db4 in operator new[](unsigned long)+0x5db4 (a.out.wasm+0x5db4)
      #1 0xb41 in __original_main use_after_free.cpp:2:16
      #2 0xc48 in main+0xc48 (a.out.wasm+0xc48)
      #3 0x800019b5 in Object.Module._main a.out.js:6581:32
      #4 0x80001ade in Object.callMain a.out.js:6878:30
      #5 0x80001b18 in doRun a.out.js:6936:60
      #6 0x80001b26 in run a.out.js:6950:5
      #7 0x80001ac9 in runCaller a.out.js:6857:29

  SUMMARY: AddressSanitizer: heap-use-after-free (a.out.wasm+0xc1a)
  ...

Memory Leaks
^^^^^^^^^^^^

Consider ``leak.cpp``:

.. code-block:: cpp

  int main() {
    new int[10];
  }

.. code-block:: console

  $ em++ -gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH -sEXIT_RUNTIME leak.cpp
  $ node a.out.js

  =================================================================
  ==42==ERROR: LeakSanitizer: detected memory leaks

  Direct leak of 40 byte(s) in 1 object(s) allocated from:
      #0 0x5ce5 in operator new[](unsigned long)+0x5ce5 (a.out.wasm+0x5ce5)
      #1 0xb24 in __original_main leak.cpp:2:3
      #2 0xb3a in main+0xb3a (a.out.wasm+0xb3a)
      #3 0x800019b8 in Object.Module._main a.out.js:6584:32
      #4 0x80001ae1 in Object.callMain a.out.js:6881:30
      #5 0x80001b1b in doRun a.out.js:6939:60
      #6 0x80001b29 in run a.out.js:6953:5
      #7 0x80001acc in runCaller a.out.js:6860:29

  SUMMARY: AddressSanitizer: 40 byte(s) leaked in 1 allocation(s).

Note that since leak checks take place at program exit, you must use
``-sEXIT_RUNTIME``, or invoke ``__lsan_do_leak_check`` or
``__lsan_do_recoverable_leak_check`` manually.

You can detect that AddressSanitizer is enabled and run ``__lsan_do_leak_check``
by doing:

.. code-block:: c

  #include <sanitizer/lsan_interface.h>

  #if defined(__has_feature)
  #if __has_feature(address_sanitizer)
    // code for ASan-enabled builds
    __lsan_do_leak_check();
  #endif
  #endif

This will be fatal if there are memory leaks. To check for memory leaks
and allow the process to continue running, use
``__lsan_do_recoverable_leak_check``.

Also, if you only want to check for memory leaks, you may use
``-fsanitize=leak`` instead of ``-fsanitize=address``. ``-fsanitize=leak``
does not instrument all memory accesses, and as a result is much faster than
``-fsanitize=address``.

Use After Return
^^^^^^^^^^^^^^^^

Consider ``use_after_return.c``:

.. code-block:: c

  #include <stdio.h>

  const char *__asan_default_options() {
    return "detect_stack_use_after_return=1";
  }

  int *f() {
    int buf[10];
    return buf;
  }

  int main() {
    *f() = 1;
  }

Note that to do this check, you have to use the ASan option
``detect_stack_use_after_return``. You may enable this option by declaring
a function called ``__asan_default_options`` like the example, or you can
define ``Module['ASAN_OPTIONS'] = 'detect_stack_use_after_return=1'`` in the
generated JavaScript. ``--pre-js`` is helpful here.

This option is fairly expensive because it converts stack allocations into
heap allocations, and these allocations are not reused so that future accesses
can cause traps. Hence, it is not enabled by default.

.. code-block:: console

  $ emcc -gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH use_after_return.c
  $ node a.out.js
  =================================================================
  ==42==ERROR: AddressSanitizer: stack-use-after-return on address 0x02a95010 at pc 0x00000d90 bp 0x02965f70 sp 0x02965f7c
  WRITE of size 4 at 0x02a95010 thread T0
      #0 0xd90 in __original_main use_after_return.c:13:10
      #1 0xe0a in main+0xe0a (a.out.wasm+0xe0a)

  Address 0x02a95010 is located in stack of thread T0 at offset 16 in frame
      #0 0x11  (a.out.wasm+0x11)

    This frame has 1 object(s):
      [16, 56) 'buf' (line 8) <== Memory access at offset 16 is inside this variable
  HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
        (longjmp and C++ exceptions *are* supported)
  SUMMARY: AddressSanitizer: stack-use-after-return (a.out.wasm+0xd8f)
  ...

Configuration
-------------

ASan can be configured via a ``--pre-js`` file:

.. code-block:: javascript

  Module.ASAN_OPTIONS = 'option1=a:option2=b';

For example, put the above snippet with your options into ``asan_options.js``,
and compile with ``--pre-js asan_options.js``.

For standalone LSan, use ``Module.LSAN_OPTIONS`` instead.

For a detailed understanding of the flags, see the `ASan documentation`__.
Please be warned that most flag combinations are not tested and may or may not
work.

__ https://github.com/google/sanitizers/wiki/AddressSanitizerFlags#run-time-flags

Disabling ``malloc``/``free`` Stack Traces
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In a program that uses ``malloc``/``free`` (or their C++ equivalent,
``operator new``/``operator delete``) very frequently, taking a stack trace at
all invocations to ``malloc``/``free`` can be very expensive. As a result, if
you find your program to be very slow when using ASan, you can try using the
option ``malloc_context_size=0``, like this:

.. code-block:: javascript

  Module.ASAN_OPTIONS = 'malloc_context_size=0';

This prevents ASan from reporting the location of memory leaks or offering
insight into where the memory for a heap-based memory error originated,
but may provide tremendous speed ups.

Comparison to ``SAFE_HEAP``
---------------------------

Emscripten provides a ``SAFE_HEAP`` mode, which can be activated by running
``emcc`` with ``-sSAFE_HEAP``. This does several things, some of which overlap
with sanitizers.

In general, ``SAFE_HEAP`` focuses on the specific pain points that come up when
targeting Wasm. The sanitizers on the other hand focus on the specific pain points that are
involved with using languages like C/C++. Those two sets overlap, but are not
identical. Which you should use depends on which types of problems you are
looking for. You may want to test with all sanitizers and with ``SAFE_HEAP``
for maximal coverage, but you may need to build separately for each mode, since
not all sanitizers are compatible with each other, and not all of them are
compatible with ``SAFE_HEAP`` (because the sanitizers do some pretty radical
things!). You will get a compiler error if there is an issue with the flags you
passed. A reasonable set of separate test builds to do might be: ASan, UBsan,
and ``SAFE_HEAP``.

The specific things ``SAFE_HEAP`` errors on include:

* **NULL pointer (address 0) reads or writes**. As mentioned earlier, this is
  annoying in WebAssembly and JavaScript because 0 is just a normal address, so
  you don't get an immediate segfault, which can be confusing.
* **Unaligned reads or writes**. These work in WebAssembly, but on some platforms
  an incorrectly-aligned read or write may be much slower, and with wasm2js
  (``WASM=0``) it will be incorrect, as JavaScript Typed Arrays do not allow
  unaligned operations.
* **Reads or writes past the top of valid memory** as managed by ``sbrk()``, that is,
  memory that was not properly allocated by ``malloc()``. This is not specific
  to Wasm, however, in JavaScript if the address is big enough to be outside the
  Typed Array, ``undefined`` is returned which can be very confusing, which is
  why this was added (in Wasm at least an error is thrown; ``SAFE_HEAP`` still
  helps with Wasm though, by checking the area between the top of ``sbrk()``'s
  memory and the end of the Wasm Memory).

``SAFE_HEAP`` does these checks by instrumenting every single load and store.
That has the cost of slowing things down, but it does give a simple guarantee
of finding *all* such problems. It can also be done after compilation, on an
arbitrary Wasm binary, while the sanitizers must be done when compiling from
source.

In comparison, UBSan can also find null pointer reads and writes. It does not
instrument every single load and store, however, as it is done during
compilation of the source code, so the checks are added where clang knows they
are needed. This is much more efficient, but there is a risk of codegen and
optimizations changing something, or clang missing a specific location.

ASan can find reads or writes of unallocated memory, which includes addresses
above the ``sbrk()``-managed memory. It may be more efficient than ``SAFE_HEAP``
in some cases: while it also checks every load and store, the LLVM
optimizer is run after it adds those checks, which can remove some of them.
