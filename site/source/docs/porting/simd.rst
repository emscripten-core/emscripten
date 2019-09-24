.. Porting SIMD code:

=======================================
Porting SIMD code targeting WebAssembly
=======================================

Emscripten supports the `WebAssembly SIMD proposal <https://github.com/webassembly/simd/>`_ when using the WebAssembly LLVM backend. To enable SIMD, pass the -msimd128 flag at compile time. This will also turn on LLVM's autovectorization passes, so no source modifications are necessary to benefit from SIMD.

At the source level, the GCC/Clang `SIMD Vector Extensions <https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html>`_ can be used and will be lowered to WebAssembly SIMD instructions where possible. In addition, there is a portable intrinsics header file that can be used.

    .. code-block:: cpp

       #include <wasm_simd128.h>

Separate documentation for the intrinsics header is a work in progress, but its usage is straightforward and its source can be found at `wasm_simd128.h <https://github.com/emscripten-core/emscripten/blob/master/system/include/wasm_simd128.h>`. These intrinsics are under active development in parallel with the SIMD proposal and should not be considered any more stable than the proposal itself.

WebAssembly SIMD is not supported when using the Fastcomp backend.

==================================
Porting SIMD code targeting asm.js
==================================

Emscripten utilizes a polyfill of the `SIMD.js specification <https://tc39.github.io/ecmascript_simd/>`_ to enable support for building SIMD code. Note that no modern JS engine supports SIMD.js natively, so using Emscripten's SIMD.js support will almost always be slower than building without SIMD. As such, Emscripten's SIMD.js support should be considered deprecated and may be removed in a future release.

There are two different ways to generate code using the SIMD.js polyfill:

- The easiest way is to utilize LLVM autovectorization support. This does not require any code changes and it is the most straightforward option to try for codebases that have not explicitly been written to target SIMD. To enable autovectorization, pass the linker flag -s SIMD=1 both when compiling and linking output files.

- Emscripten supports the GCC/Clang compiler specific `SIMD Vector Extensions <https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html>`_. These constructs do not require any changes to the command line build flags, but any code that utilizes the vector built-ins will always unconditionally emit SIMD.js vector instructions.

======================================
Limitations and behavioral differences
======================================

When porting native SIMD code, it should be noted that because of portability concerns, neither the WebAssembly SIMD proposal nor SIMD.js expose the full native instruction sets. In particular the following changes exist:

 - Emscripten does not support x86 or any other native inline SIMD assembly or building .s assembly files, so all code should be written to use SIMD intrinsic functions or compiler vector extensions.

 - WebAssembly SIMD and SIMD.js do not have control over managing floating point rounding modes or handling denormals.

 - Cache line prefetch instructions are not available, and calls to these functions will compile, but be treated as no-ops.

 - Asymmetric memory fence operations are not available, but will be implemented as fully synchronous memory fences when SharedArrayBuffer is enabled (-s USE_PTHREADS=1) or as no-ops when multithreading is not enabled (default, -s USE_PTHREADS=0).

SIMD-related bug reports are tracked in the `Emscripten bug tracker with the label SIMD <https://github.com/emscripten-core/emscripten/issues?q=is%3Aopen+is%3Aissue+label%3ASIMD>`_.
