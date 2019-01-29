.. Porting SIMD code:

==============================
Porting SIMD code
==============================

Emscripten utilizes the `SIMD.js specification <https://tc39.github.io/ecmascript_simd/>`_ to enable support for building SIMD code. While the specification does not expose direct access to native CPU SIMD instructions, it does offer the most basic operations that codebases commonly need. Given the nature of the SIMD.js specification, compiled SIMD code is platform-abstract and the generated .js files will run accelerated on both x86 SSE and ARM NEON enabled CPUs (and any other vector instruction sets that browsers might be targeting), although performance can vary.

For backwards compatibility, all generated SIMD code will embed a polyfill for all SIMD.js instructions. This allows Emscripten compiled pages to be run even on browsers that don't have SIMD support available. However, due to the nature of the interoperation with asm.js, the SIMD.js polyfill will likely be much slower than scalar execution, so it is best to primarily target browsers that do support SIMD.js in hardware.

Enabling SIMD code generation
=============================

There are three different ways to generate code to benefit from SIMD instructions:

- The easiest way is to utilize LLVM autovectorization support. This does not require any code changes and it is the most straightforward option to try for codebases that have not explicitly been written to target SIMD. To enable autovectorization, pass the linker flag -s SIMD=1 both when compiling and linking output files. Note that not all code can be autovectorized in this manner by the compiler, so the performance gains will greatly depend on the structure of the code in its hot paths.

- Emscripten supports the GCC/Clang compiler specific `SIMD Vector Extensions <https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html>`_. These constructs do not require any changes to the command line build flags, but any code that utilizes the vector built-ins will always unconditionally emit SIMD.js vector instructions.

- A third option is to use the x86 SSE intrinsics. Emscripten has full support for compiling code that utilizes the SSE1, SSE2, SSE3 and SSSE3 intrinsic function calls. To enable SSE1 intrinsics support, pass the compiler flag -msse, and add in a #include <xmmintrin.h>. To build SSE2 intrinsics code, pass the compiler flag -msse2, and use #include <emmintrin.h>. For SSE3, pass -msse3 and #include <pmmintrin.h>, and for SSSE3, pass -mssse3 and #include <tmmintrin.h>.

These three methods are not mutually exclusive, but may freely be combined.

Limitations and behavioral differences
======================================

When porting native SIMD code, it should be noted that because of portability concerns, SIMD.js does not expose the full native instruction sets to JavaScript. In particular the following changes exist:

 - Emscripten does not support x86 or any other native inline SIMD assembly or building .s assembly files, but all code should be written to use SIMD intrinsic functions.

 - The SIMD types supported by SIMD.js are Float32x4, Int32x4, Uint32x4, Int16x8, Uint16x8, Int8x16 and Uint8x16. In particular, Float64x2 and Int64x2 are currently not supported, however Float64x2 is emulated in software in the current polyfill. 256-bit or wider SIMD types (AVX) are not supported either.

 - Even though the full set of SSE1, SSE2, SSE3 and SSSE3 intrinsics are supported, because of the platform-abstract nature of SIMD.js, some of these intrinsics will compile down to scalarized instructions to emulate. To verify which instructions are accelerated and which are not, examine the code in the platform headers `xmmintrin.h <https://github.com/emscripten-core/emscripten/blob/incoming/system/include/emscripten/xmmintrin.h>`_ and `emmintrin.h <https://github.com/emscripten-core/emscripten/blob/incoming/system/include/emscripten/xmmintrin.h>`_.

 - Currently the Intel x86 SIMD support is limited to SSE1, SSE2, SSE3 and SSSE3 instruction sets. The Intel x86 SSE4.1, SSE4.2, AVX, AVX2 and FMA instruction sets or newer are not supported. Also, the old Intel x86 MMX instruction set is not supported.

 - SIMD.js does not have control over managing floating point rounding modes or handling denormals.

 - Cache line prefetch instructions are not available, and calls to these functions will compile, but be treated as no-ops.

 - Asymmetric memory fence operations are not available, but will be implemented as fully synchronous memory fences when SharedArrayBuffer is enabled (-s USE_PTHREADS=1) or as no-ops when multithreading is not enabled (default, -s USE_PTHREADS=0).

 - Building ARM NEON -based intrinsics (#include <arm_neon.h>) code is not currently supported, but could be doable. Contributions on this front are welcome.

Tests and Benchmarks
====================

Emscripten repository has several tests for SIMD support. To run SIMD tests, execute e.g. "python tests/runner.py asm*.test_sse1_full" in Emscripten root directory (note that currently - 2018-7-11 - only asm.js supports SIMD, so we run the `asm*` tests). For the full list of tests, see test_simd* and test_sse* in `test_core.py <https://github.com/emscripten-core/emscripten/blob/incoming/tests/test_core.py>`_.

To run a synthetic SSE1 API benchmark, execute "python tests/benchmark_sse1.py" in Emscripten root directory.

SIMD-related bug reports are tracked in the `Emscripten bug tracker with the label SIMD <https://github.com/emscripten-core/emscripten/issues?q=is%3Aopen+is%3Aissue+label%3ASIMD>`_.
