.. Porting SIMD code:

=======================================
Porting SIMD code targeting WebAssembly
=======================================

Emscripten supports the `WebAssembly SIMD proposal <https://github.com/webassembly/simd/>`_ when using the WebAssembly LLVM backend. To enable SIMD, pass the -msimd128 flag at compile time. This will also turn on LLVM's autovectorization passes, so no source modifications are necessary to benefit from SIMD.

At the source level, the GCC/Clang `SIMD Vector Extensions <https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html>`_ can be used and will be lowered to WebAssembly SIMD instructions where possible. In addition, there is a portable intrinsics header file that can be used.

    .. code-block:: cpp

       #include <wasm_simd128.h>

Separate documentation for the intrinsics header is a work in progress, but its usage is straightforward and its source can be found at `wasm_simd128.h <https://github.com/llvm/llvm-project/blob/master/clang/lib/Headers/wasm_simd128.h>`_. These intrinsics are under active development in parallel with the SIMD proposal and should not be considered any more stable than the proposal itself. Note that most engines will also require an extra flag to enable SIMD. For example, Node requires `--experimental-wasm-simd`.

WebAssembly SIMD is not supported when using the Fastcomp backend.

======================================
Limitations and behavioral differences
======================================

When porting native SIMD code, it should be noted that because of portability concerns, the WebAssembly SIMD proposal does not expose the full native instruction sets. In particular the following changes exist:

 - Emscripten does not support x86 or any other native inline SIMD assembly or building .s assembly files, so all code should be written to use SIMD intrinsic functions or compiler vector extensions.

 - WebAssembly SIMD does not have control over managing floating point rounding modes or handling denormals.

 - Cache line prefetch instructions are not available, and calls to these functions will compile, but be treated as no-ops.

 - Asymmetric memory fence operations are not available, but will be implemented as fully synchronous memory fences when SharedArrayBuffer is enabled (-s USE_PTHREADS=1) or as no-ops when multithreading is not enabled (default, -s USE_PTHREADS=0).

SIMD-related bug reports are tracked in the `Emscripten bug tracker with the label SIMD <https://github.com/emscripten-core/emscripten/issues?q=is%3Aopen+is%3Aissue+label%3ASIMD>`_.
