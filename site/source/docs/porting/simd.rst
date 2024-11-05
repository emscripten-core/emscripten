.. Porting SIMD code:

.. role:: raw-html(raw)
    :format: html

===========================
Using SIMD with WebAssembly
===========================

Emscripten supports the `WebAssembly SIMD <https://github.com/webassembly/simd/>`_ feature. There are five different ways to leverage WebAssembly SIMD in your C/C++ programs:

1. Enable LLVM/Clang SIMD autovectorizer to automatically target WebAssembly SIMD, without requiring changes to C/C++ source code.
2. Write SIMD code using the GCC/Clang SIMD Vector Extensions (``__attribute__((vector_size(16)))``)
3. Write SIMD code using the WebAssembly SIMD intrinsics (``#include <wasm_simd128.h>``)
4. Compile existing SIMD code that uses the x86 SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2 or AVX intrinsics (``#include <*mmintrin.h>``)
5. Compile existing SIMD code that uses the ARM NEON intrinsics (``#include <arm_neon.h>``)

These techniques can be freely combined in a single program.

To enable any of the five types of SIMD above, pass the WebAssembly-specific ``-msimd128`` flag at compile time. This will also turn on LLVM's autovectorization passes. If that is not desirable, additionally pass flags ``-fno-vectorize -fno-slp-vectorize`` to disable the autovectorizer. See `Auto-Vectorization in LLVM <https://llvm.org/docs/Vectorizers.html>`_ for more information.

WebAssembly SIMD is supported by

* Chrome ≥ 91 (May 2021),

* Firefox ≥ 89 (June 2021),

* Safari ≥ 16.4 (March 2023) and

* Node.js ≥ 16.4 (June 2021).

See `WebAssembly Roadmap <https://webassembly.org/roadmap/>`_ for details about other VMs.

An upcoming `Relaxed SIMD proposal <https://github.com/WebAssembly/relaxed-simd/tree/main/proposals/relaxed-simd>`_ will add more SIMD instructions to WebAssembly.

================================
GCC/Clang SIMD Vector Extensions
================================

At the source level, the GCC/Clang `SIMD Vector Extensions <https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html>`_ can be used and will be lowered to WebAssembly SIMD instructions where possible.

This enables developers to create custom wide vector types via typedefs, and use arithmetic operators (+,-,*,/) on the vectorized types, as well as allow individual lane access via the vector[i] notation. However, the `GCC vector built-in functions <https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html>`_ are not available. Instead, use the WebAssembly SIMD Intrinsics functions below.

===========================
WebAssembly SIMD Intrinsics
===========================

LLVM maintains a WebAssembly SIMD Intrinsics header file that is provided with Emscripten, and adds type definitions for the different supported vector types.

    .. code-block:: cpp

       #include <wasm_simd128.h>
       #include <stdio.h>

       int main() {
       #ifdef __wasm_simd128__
         v128_t v1 = wasm_f32x4_make(1.2f, 3.4f, 5.6f, 7.8f);
         v128_t v2 = wasm_f32x4_make(2.1f, 4.3f, 6.5f, 8.7f);
         v128_t v3 = wasm_f32x4_add(v1, v2);
         // Prints "v3: [3.3, 7.7, 12.1, 16.5]"
         printf("v3: [%.1f, %.1f, %.1f, %.1f]\n",
                wasm_f32x4_extract_lane(v3, 0),
                wasm_f32x4_extract_lane(v3, 1),
                wasm_f32x4_extract_lane(v3, 2),
                wasm_f32x4_extract_lane(v3, 3));
       #endif
       }

The Wasm SIMD header can be browsed online at `wasm_simd128.h <https://github.com/llvm/llvm-project/blob/main/clang/lib/Headers/wasm_simd128.h>`_.

Pass flag ``-msimd128`` at compile time to enable targeting WebAssembly SIMD Intrinsics. C/C++ code can use the built-in preprocessor define ``#ifdef __wasm_simd128__`` to detect when building with WebAssembly SIMD enabled.

Pass ``-mrelaxed-simd`` to target WebAssembly Relaxed SIMD Intrinsics. C/C++ code can use the built-in preprocessor define ``#ifdef __wasm_relaxed_simd__`` to detect when this target is active.

======================================
Limitations and behavioral differences
======================================

When porting native SIMD code, it should be noted that because of portability concerns, the WebAssembly SIMD specification does not expose access to all of the native x86/ARM SIMD instructions. In particular the following changes exist:

 - Emscripten does not support x86 or any other native inline SIMD assembly or building .s assembly files, so all code should be written to use SIMD intrinsic functions or compiler vector extensions.

 - WebAssembly SIMD does not have control over managing floating point rounding modes or handling denormals.

 - Cache line prefetch instructions are not available, and calls to these functions will compile, but are treated as no-ops.

 - Asymmetric memory fence operations are not available, but will be implemented as fully synchronous memory fences when SharedArrayBuffer is enabled (-pthread) or as no-ops when multithreading is not enabled (the default).

SIMD-related bug reports are tracked in the `Emscripten bug tracker with the label SIMD <https://github.com/emscripten-core/emscripten/issues?q=is%3Aopen+is%3Aissue+label%3ASIMD>`_.

===========================
Optimization considerations
===========================

When developing SIMD code to use WebAssembly SIMD, implementors should be aware of semantic differences between the host hardware and WebAssembly semantics; as acknowledged in the WebAssembly design documentation, "`this sometimes will lead to poor performance <https://github.com/WebAssembly/design/blob/master/Portability.md#assumptions-for-efficient-execution>`_." The following list outlines some WebAssembly SIMD instructions to look out for when performance tuning:

.. list-table:: WebAssembly SIMD instructions with performance implications
   :widths: 10 10 30
   :header-rows: 1

   * - WebAssembly SIMD instruction
     - Arch
     - Considerations

   * - [i8x16|i16x8|i32x4|i64x2].[shl|shr_s|shr_u]
     - x86, arm
     - Use a constant shift amount to avoid extra instructions checking that it is in bounds.

   * - i8x16.[shl|shr_s|shr_u]
     - x86
     - Included for orthogonality, these instructions have no equivalent x86 instruction and are emulated with `5-11 x86 instructions in v8 <https://github.com/v8/v8/blob/c8672adeebb105c7636334b9931831bf1945f4ec/src/codegen/shared-ia32-x64/macro-assembler-shared-ia32-x64.cc#L427-L552>`_ (i.e. using 16x8 shifts).
  
   * - i64x2.shr_s
     - x86
     - Included for orthogonality, this instruction has no equivalent x86 instruction and is emulated with `6-12 x86 instructions in v8 <https://github.com/v8/v8/blob/c8672adeebb105c7636334b9931831bf1945f4ec/src/codegen/shared-ia32-x64/macro-assembler-shared-ia32-x64.cc#L996-L1057>`_.

   * - i8x16.swizzle
     - x86
     - The zeroing behavior does not match x86 (i.e. this instruction zeroes when an index is out-of-range instead of when the most significant bit is 1); use a constant swizzle amount (or i8x16.shuffle) to avoid 3 extra x86 instructions in some runtimes.

   * - [f32x4|f64x2].[min|max]
     - x86
     - As with the scalar versions, the NaN propagation semantics force runtimes to emulate with 7-10 x86 instructions (e.g., see `v8's emulation <https://github.com/v8/v8/blob/c8672adeebb105c7636334b9931831bf1945f4ec/src/codegen/shared-ia32-x64/macro-assembler-shared-ia32-x64.cc#L202-L339>`_; if possible, use [f32x4|f64x2].[pmin|pmax] instead (1 x86 instruction).

   * - i32x4.trunc_sat_f32x4_[u|s]
     - x86
     - No equivalent x86 semantics; `emulated with 8-14 x86 instructions in v8 <https://github.com/v8/v8/blob/b6520eda5eafc3b007a5641b37136dfc9d92f63d/src/compiler/backend/x64/code-generator-x64.cc#L3035-L3062>`_.

   * - i32x4.trunc_sat_f64x2_[u|s]_zero
     - x86
     - No equivalent x86 semantics; `emulated with 5-6 x86 instructions in v8 <https://github.com/v8/v8/blob/b6520eda5eafc3b007a5641b37136dfc9d92f63d/src/codegen/x64/macro-assembler-x64.cc#L2241-L2311>`_.

   * - f32x4.convert_f32x4_u
     - x86
     - No equivalent x86 semantics; `emulated with 8 x86 instructions in v8 <https://github.com/v8/v8/blob/b6520eda5eafc3b007a5641b37136dfc9d92f63d/src/compiler/backend/x64/code-generator-x64.cc#L2591-L2604>`_.

   * - [i8x16|i64x2].mul
     - x86
     - Included for orthogonality, these instructions have no equivalent x86 instruction and are `emulated with 10 x86 instructions in v8 <https://github.com/v8/v8/blob/b6520eda5eafc3b007a5641b37136dfc9d92f63d/src/compiler/backend/x64/code-generator-x64.cc#L2834-L2858>`_.


=======================================================
Compiling SIMD code targeting x86 SSE* instruction sets
=======================================================

Emscripten supports compiling existing codebases that use x86 SSE instructions by passing the ``-msimd128`` flag, and additionally one of the following:

* **SSE**: pass ``-msse`` and ``#include <xmmintrin.h>``. Use ``#ifdef __SSE__`` to gate code.
* **SSE2**: pass ``-msse2`` and ``#include <emmintrin.h>``. Use ``#ifdef __SSE2__`` to gate code.
* **SSE3**: pass ``-msse3`` and ``#include <pmmintrin.h>``. Use ``#ifdef __SSE3__`` to gate code.
* **SSSE3**: pass ``-mssse3`` and ``#include <tmmintrin.h>``. Use ``#ifdef __SSSE3__`` to gate code.
* **SSE4.1**: pass ``-msse4.1`` and ``#include <smmintrin.h>``. Use ``#ifdef __SSE4_1__`` to gate code.
* **SSE4.2**: pass ``-msse4.2`` and ``#include <nmmintrin.h>``. Use ``#ifdef __SSE4_2__`` to gate code.
* **AVX**: pass ``-mavx`` and ``#include <immintrin.h>``. Use ``#ifdef __AVX__`` to gate code.

Currently only the SSE1, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, and AVX instruction sets are supported. Each of these instruction sets add on top of the previous ones, so e.g. when targeting SSE3, the instruction sets SSE1 and SSE2 are also available.

The following tables highlight the availability and expected performance of different SSE* intrinsics. This can be useful for understanding the performance limitations that the Wasm SIMD specification has when running on x86 hardware.

For detailed information on each SSE intrinsic function, visit the excellent `Intel Intrinsics Guide on SSE1 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE>`_.

The following legend is used to highlight the expected performance of various instructions:
 - ✅ Wasm SIMD has a native opcode that matches the x86 SSE instruction, should yield native performance
 - 💡 while the Wasm SIMD spec does not provide a proper performance guarantee, given a suitably smart enough compiler and a runtime VM path, this intrinsic should be able to generate the identical native SSE instruction.
 - 🟡 there is some information missing (e.g. type or alignment information) for a Wasm VM to be guaranteed to be able to reconstruct the intended x86 SSE opcode. This might cause a penalty depending on the target CPU hardware family, especially on older CPU generations.
 - ⚠️ the underlying x86 SSE instruction is not available, but it is emulated via at most few other Wasm SIMD instructions, causing a small penalty.
 - ❌ the underlying x86 SSE instruction is not exposed by the Wasm SIMD specification, so it must be emulated via a slow path, e.g. a sequence of several slower SIMD instructions, or a scalar implementation.
 - 💣 the underlying x86 SSE opcode is not available in Wasm SIMD, and the implementation must resort to such a slow emulated path, that a workaround rethinking the algorithm at a higher level is advised.
 - 💭 the given SSE intrinsic is available to let applications compile, but does nothing.
 - ⚫ the given SSE intrinsic is not available. Referencing the intrinsic will cause a compiler error.

Certain intrinsics in the table below are marked "virtual". This means that there does not actually exist a native x86 SSE instruction set opcode to implement them, but native compilers offer the function as a convenience. Different compilers might generate a different instruction sequence for these.

In addition to consulting the tables below, you can turn on diagnostics for slow, emulated functions by defining the macro ``#define WASM_SIMD_COMPAT_SLOW``. This will print out warnings if you attempt to use any of the slow paths (corresponding to ❌ or 💣 in the legend).

.. list-table:: x86 SSE intrinsics available via #include <xmmintrin.h> and -msse
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_set_ps
     - ✅ wasm_f32x4_make
   * - _mm_setr_ps
     - ✅ wasm_f32x4_make
   * - _mm_set_ss
     - 💡 emulated with wasm_f32x4_make
   * - _mm_set_ps1 (_mm_set1_ps)
     - ✅ wasm_f32x4_splat
   * - _mm_setzero_ps
     - 💡 emulated with wasm_f32x4_const(0)
   * - _mm_load_ps
     - 🟡 wasm_v128_load. VM must guess type. :raw-html:`<br />` Unaligned load on x86 CPUs.
   * - _mm_loadl_pi
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with scalar loads + shuffle.
   * - _mm_loadh_pi
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with scalar loads + shuffle.
   * - _mm_loadr_ps
     - 💡 Virtual. Simd load + shuffle.
   * - _mm_loadu_ps
     - 🟡 wasm_v128_load. VM must guess type.
   * - _mm_load_ps1 (_mm_load1_ps)
     - 🟡 Virtual. Simd load + shuffle.
   * - _mm_load_ss
     - ❌ emulated with wasm_f32x4_make
   * - _mm_storel_pi
     - ❌ scalar stores
   * - _mm_storeh_pi
     - ❌ shuffle + scalar stores
   * - _mm_store_ps
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` Unaligned store on x86 CPUs.
   * - _mm_stream_ps
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` No cache control in Wasm SIMD.
   * - _mm_prefetch
     - 💭 No-op.
   * - _mm_sfence
     - ⚠️ A full barrier in multithreaded builds.
   * - _mm_shuffle_ps
     - 🟡 wasm_i32x4_shuffle. VM must guess type.
   * - _mm_storer_ps
     - 💡 Virtual. Shuffle + Simd store.
   * - _mm_store_ps1 (_mm_store1_ps)
     - 🟡 Virtual. Emulated with shuffle. :raw-html:`<br />` Unaligned store on x86 CPUs.
   * - _mm_store_ss
     - 💡 emulated with scalar store
   * - _mm_storeu_ps
     - 🟡 wasm_v128_store. VM must guess type.
   * - _mm_storeu_si16
     - 💡 emulated with scalar store
   * - _mm_storeu_si64
     - 💡 emulated with scalar store
   * - _mm_movemask_ps
     - ✅ wasm_i32x4_bitmask
   * - _mm_move_ss
     - 💡 emulated with a shuffle. VM must guess type.
   * - _mm_add_ps
     - ✅ wasm_f32x4_add
   * - _mm_add_ss
     - ⚠️ emulated with a shuffle
   * - _mm_sub_ps
     - ✅ wasm_f32x4_sub
   * - _mm_sub_ss
     - ⚠️ emulated with a shuffle
   * - _mm_mul_ps
     - ✅ wasm_f32x4_mul
   * - _mm_mul_ss
     - ⚠️ emulated with a shuffle
   * - _mm_div_ps
     - ✅ wasm_f32x4_div
   * - _mm_div_ss
     - ⚠️ emulated with a shuffle
   * - _mm_min_ps
     - TODO: pmin once it works
   * - _mm_min_ss
     - ⚠️ emulated with a shuffle
   * - _mm_max_ps
     - TODO: pmax once it works
   * - _mm_max_ss
     - ⚠️ emulated with a shuffle
   * - _mm_rcp_ps
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with full precision div. `simd/#3 <https://github.com/WebAssembly/simd/issues/3>`_
   * - _mm_rcp_ss
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with full precision div+shuffle `simd/#3 <https://github.com/WebAssembly/simd/issues/3>`_
   * - _mm_sqrt_ps
     - ✅ wasm_f32x4_sqrt
   * - _mm_sqrt_ss
     - ⚠️ emulated with a shuffle
   * - _mm_rsqrt_ps
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with full precision div+sqrt. `simd/#3 <https://github.com/WebAssembly/simd/issues/3>`_
   * - _mm_rsqrt_ss
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with full precision div+sqrt+shuffle. `simd/#3 <https://github.com/WebAssembly/simd/issues/3>`_
   * - _mm_unpackhi_ps
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_ps
     - 💡 emulated with a shuffle
   * - _mm_movehl_ps
     - 💡 emulated with a shuffle
   * - _mm_movelh_ps
     - 💡 emulated with a shuffle
   * - _MM_TRANSPOSE4_PS
     - 💡 emulated with a shuffle
   * - _mm_cmplt_ps
     - ✅ wasm_f32x4_lt
   * - _mm_cmplt_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmple_ps
     - ✅ wasm_f32x4_le
   * - _mm_cmple_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpeq_ps
     - ✅ wasm_f32x4_eq
   * - _mm_cmpeq_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpge_ps
     - ✅ wasm_f32x4_ge
   * - _mm_cmpge_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpgt_ps
     - ✅ wasm_f32x4_gt
   * - _mm_cmpgt_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpord_ps
     - ❌ emulated with 2xcmp+and
   * - _mm_cmpord_ss
     - ❌ emulated with 2xcmp+and+shuffle
   * - _mm_cmpunord_ps
     - ❌ emulated with 2xcmp+or
   * - _mm_cmpunord_ss
     - ❌ emulated with 2xcmp+or+shuffle
   * - _mm_and_ps
     - 🟡 wasm_v128_and. VM must guess type.
   * - _mm_andnot_ps
     - 🟡 wasm_v128_andnot. VM must guess type.
   * - _mm_or_ps
     - 🟡 wasm_v128_or. VM must guess type.
   * - _mm_xor_ps
     - 🟡 wasm_v128_xor. VM must guess type.
   * - _mm_cmpneq_ps
     - ✅ wasm_f32x4_ne
   * - _mm_cmpneq_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpnge_ps
     - ⚠️ emulated with not+ge
   * - _mm_cmpnge_ss
     - ⚠️ emulated with not+ge+shuffle
   * - _mm_cmpngt_ps
     - ⚠️ emulated with not+gt
   * - _mm_cmpngt_ss
     - ⚠️ emulated with not+gt+shuffle
   * - _mm_cmpnle_ps
     - ⚠️ emulated with not+le
   * - _mm_cmpnle_ss
     - ⚠️ emulated with not+le+shuffle
   * - _mm_cmpnlt_ps
     - ⚠️ emulated with not+lt
   * - _mm_cmpnlt_ss
     - ⚠️ emulated with not+lt+shuffle
   * - _mm_comieq_ss
     - ❌ scalarized
   * - _mm_comige_ss
     - ❌ scalarized
   * - _mm_comigt_ss
     - ❌ scalarized
   * - _mm_comile_ss
     - ❌ scalarized
   * - _mm_comilt_ss
     - ❌ scalarized
   * - _mm_comineq_ss
     - ❌ scalarized
   * - _mm_ucomieq_ss
     - ❌ scalarized
   * - _mm_ucomige_ss
     - ❌ scalarized
   * - _mm_ucomigt_ss
     - ❌ scalarized
   * - _mm_ucomile_ss
     - ❌ scalarized
   * - _mm_ucomilt_ss
     - ❌ scalarized
   * - _mm_ucomineq_ss
     - ❌ scalarized
   * - _mm_cvtsi32_ss (_mm_cvt_si2ss)
     - ❌ scalarized
   * - _mm_cvtss_si32 (_mm_cvt_ss2si)
     - 💣 scalar with complex emulated semantics
   * - _mm_cvttss_si32 (_mm_cvtt_ss2si)
     - 💣 scalar with complex emulated semantics
   * - _mm_cvtsi64_ss
     - ❌ scalarized
   * - _mm_cvtss_si64
     - 💣 scalar with complex emulated semantics
   * - _mm_cvttss_si64
     - 💣 scalar with complex emulated semantics
   * - _mm_cvtss_f32
     - 💡 scalar get
   * - _mm_malloc
     - ✅ Allocates memory with specified alignment.
   * - _mm_free
     - ✅ Aliases to free().
   * - _MM_GET_EXCEPTION_MASK
     - ✅ Always returns all exceptions masked (0x1f80).
   * - _MM_GET_EXCEPTION_STATE
     - ❌ Exception state is not tracked. Always returns 0.
   * - _MM_GET_FLUSH_ZERO_MODE
     - ✅ Always returns _MM_FLUSH_ZERO_OFF.
   * - _MM_GET_ROUNDING_MODE
     - ✅ Always returns _MM_ROUND_NEAREST.
   * - _mm_getcsr
     - ✅ Always returns _MM_FLUSH_ZERO_OFF :raw-html:`<br />` | _MM_ROUND_NEAREST | all exceptions masked (0x1f80).
   * - _MM_SET_EXCEPTION_MASK
     - ⚫ Not available. Fixed to all exceptions masked.
   * - _MM_SET_EXCEPTION_STATE
     - ⚫ Not available. Fixed to zero/clear state.
   * - _MM_SET_FLUSH_ZERO_MODE
     - ⚫ Not available. Fixed to _MM_FLUSH_ZERO_OFF.
   * - _MM_SET_ROUNDING_MODE
     - ⚫ Not available. Fixed to _MM_ROUND_NEAREST.
   * - _mm_setcsr
     - ⚫ Not available.
   * - _mm_undefined_ps
     - ✅ Virtual

⚫ The following extensions that SSE1 instruction set brought to 64-bit wide MMX registers are not available:
 - _mm_avg_pu8, _mm_avg_pu16, _mm_cvt_pi2ps, _mm_cvt_ps2pi, _mm_cvt_pi16_ps, _mm_cvt_pi32_ps, _mm_cvt_pi32x2_ps, _mm_cvt_pi8_ps, _mm_cvt_ps_pi16, _mm_cvt_ps_pi32, _mm_cvt_ps_pi8, _mm_cvt_pu16_ps, _mm_cvt_pu8_ps, _mm_cvtt_ps2pi, _mm_cvtt_pi16_ps, _mm_cvttps_pi32, _mm_extract_pi16, _mm_insert_pi16, _mm_maskmove_si64, _m_maskmovq, _mm_max_pi16, _mm_max_pu8, _mm_min_pi16, _mm_min_pu8, _mm_movemask_pi8, _mm_mulhi_pu16, _m_pavgb, _m_pavgw, _m_pextrw, _m_pinsrw, _m_pmaxsw, _m_pmaxub, _m_pminsw, _m_pminub, _m_pmovmskb, _m_pmulhuw, _m_psadbw, _m_pshufw, _mm_sad_pu8, _mm_shuffle_pi16 and _mm_stream_pi.

Any code referencing these intrinsics will not compile.

The following table highlights the availability and expected performance of different SSE2 intrinsics. Refer to `Intel Intrinsics Guide on SSE2 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE2>`_.

.. list-table:: x86 SSE2 intrinsics available via #include <emmintrin.h> and -msse2
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_add_epi16
     - ✅ wasm_i16x8_add
   * - _mm_add_epi32
     - ✅ wasm_i32x4_add
   * - _mm_add_epi64
     - ✅ wasm_i64x2_add
   * - _mm_add_epi8
     - ✅ wasm_i8x16_add
   * - _mm_add_pd
     - ✅ wasm_f64x2_add
   * - _mm_add_sd
     - ⚠️ emulated with a shuffle
   * - _mm_adds_epi16
     - ✅ wasm_i16x8_add_sat
   * - _mm_adds_epi8
     - ✅ wasm_i8x16_add_sat
   * - _mm_adds_epu16
     - ✅ wasm_u16x8_add_sat
   * - _mm_adds_epu8
     - ✅ wasm_u8x16_add_sat
   * - _mm_and_pd
     - 🟡 wasm_v128_and. VM must guess type.
   * - _mm_and_si128
     - 🟡 wasm_v128_and. VM must guess type.
   * - _mm_andnot_pd
     - 🟡 wasm_v128_andnot. VM must guess type.
   * - _mm_andnot_si128
     - 🟡 wasm_v128_andnot. VM must guess type.
   * - _mm_avg_epu16
     - ✅ wasm_u16x8_avgr
   * - _mm_avg_epu8
     - ✅ wasm_u8x16_avgr
   * - _mm_castpd_ps
     - ✅ no-op
   * - _mm_castpd_si128
     - ✅ no-op
   * - _mm_castps_pd
     - ✅ no-op
   * - _mm_castps_si128
     - ✅ no-op
   * - _mm_castsi128_pd
     - ✅ no-op
   * - _mm_castsi128_ps
     - ✅ no-op
   * - _mm_clflush
     - 💭 No-op. No cache hinting in Wasm SIMD.
   * - _mm_cmpeq_epi16
     - ✅ wasm_i16x8_eq
   * - _mm_cmpeq_epi32
     - ✅ wasm_i32x4_eq
   * - _mm_cmpeq_epi8
     - ✅ wasm_i8x16_eq
   * - _mm_cmpeq_pd
     - ✅ wasm_f64x2_eq
   * - _mm_cmpeq_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmpge_pd
     - ✅ wasm_f64x2_ge
   * - _mm_cmpge_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmpgt_epi16
     - ✅ wasm_i16x8_gt
   * - _mm_cmpgt_epi32
     - ✅ wasm_i32x4_gt
   * - _mm_cmpgt_epi8
     - ✅ wasm_i8x16_gt
   * - _mm_cmpgt_pd
     - ✅ wasm_f64x2_gt
   * - _mm_cmpgt_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmple_pd
     - ✅ wasm_f64x2_le
   * - _mm_cmple_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmplt_epi16
     - ✅ wasm_i16x8_lt
   * - _mm_cmplt_epi32
     - ✅ wasm_i32x4_lt
   * - _mm_cmplt_epi8
     - ✅ wasm_i8x16_lt
   * - _mm_cmplt_pd
     - ✅ wasm_f64x2_lt
   * - _mm_cmplt_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmpneq_pd
     - ✅ wasm_f64x2_ne
   * - _mm_cmpneq_sd
     - ⚠️ emulated with a shuffle
   * - _mm_cmpnge_pd
     - ⚠️ emulated with not+ge
   * - _mm_cmpnge_sd
     - ⚠️ emulated with not+ge+shuffle
   * - _mm_cmpngt_pd
     - ⚠️ emulated with not+gt
   * - _mm_cmpngt_sd
     - ⚠️ emulated with not+gt+shuffle
   * - _mm_cmpnle_pd
     - ⚠️ emulated with not+le
   * - _mm_cmpnle_sd
     - ⚠️ emulated with not+le+shuffle
   * - _mm_cmpnlt_pd
     - ⚠️ emulated with not+lt
   * - _mm_cmpnlt_sd
     - ⚠️ emulated with not+lt+shuffle
   * - _mm_cmpord_pd
     - ❌ emulated with 2xcmp+and
   * - _mm_cmpord_sd
     - ❌ emulated with 2xcmp+and+shuffle
   * - _mm_cmpunord_pd
     - ❌ emulated with 2xcmp+or
   * - _mm_cmpunord_sd
     - ❌ emulated with 2xcmp+or+shuffle
   * - _mm_comieq_sd
     - ❌ scalarized
   * - _mm_comige_sd
     - ❌ scalarized
   * - _mm_comigt_sd
     - ❌ scalarized
   * - _mm_comile_sd
     - ❌ scalarized
   * - _mm_comilt_sd
     - ❌ scalarized
   * - _mm_comineq_sd
     - ❌ scalarized
   * - _mm_cvtepi32_pd
     - ✅ wasm_f64x2_convert_low_i32x4
   * - _mm_cvtepi32_ps
     - ✅ wasm_f32x4_convert_i32x4
   * - _mm_cvtpd_epi32
     - ❌ scalarized
   * - _mm_cvtpd_ps
     - ✅ wasm_f32x4_demote_f64x2_zero
   * - _mm_cvtps_epi32
     - ❌ scalarized
   * - _mm_cvtps_pd
     - ✅ wasm_f64x2_promote_low_f32x4
   * - _mm_cvtsd_f64
     - ✅ wasm_f64x2_extract_lane
   * - _mm_cvtsd_si32
     - ❌ scalarized
   * - _mm_cvtsd_si64
     - ❌ scalarized
   * - _mm_cvtsd_si64x
     - ❌ scalarized
   * - _mm_cvtsd_ss
     - ❌ scalarized
   * - _mm_cvtsi128_si32
     - ✅ wasm_i32x4_extract_lane
   * - _mm_cvtsi128_si64 (_mm_cvtsi128_si64x)
     - ✅ wasm_i64x2_extract_lane
   * - _mm_cvtsi32_sd
     - ❌ scalarized
   * - _mm_cvtsi32_si128
     - 💡 emulated with wasm_i32x4_make
   * - _mm_cvtsi64_sd (_mm_cvtsi64x_sd)
     - ❌ scalarized
   * - _mm_cvtsi64_si128 (_mm_cvtsi64x_si128)
     - 💡 emulated with wasm_i64x2_make
   * - _mm_cvtss_sd
     - ❌ scalarized
   * - _mm_cvttpd_epi32
     - ❌ scalarized
   * - _mm_cvttps_epi32
     - ❌ scalarized
   * - _mm_cvttsd_si32
     - ❌ scalarized
   * - _mm_cvttsd_si64 (_mm_cvttsd_si64x)
     - ❌ scalarized
   * - _mm_div_pd
     - ✅ wasm_f64x2_div
   * - _mm_div_sd
     - ⚠️ emulated with a shuffle
   * - _mm_extract_epi16
     - ✅ wasm_u16x8_extract_lane
   * - _mm_insert_epi16
     - ✅ wasm_i16x8_replace_lane
   * - _mm_lfence
     - ⚠️ A full barrier in multithreaded builds.
   * - _mm_load_pd
     - 🟡 wasm_v128_load. VM must guess type. :raw-html:`<br />` Unaligned load on x86 CPUs.
   * - _mm_load1_pd (_mm_load_pd1)
     - 🟡 Virtual. wasm_v64x2_load_splat, VM must guess type.
   * - _mm_load_sd
     - ❌ emulated with wasm_f64x2_make
   * - _mm_load_si128
     - 🟡 wasm_v128_load. VM must guess type. :raw-html:`<br />` Unaligned load on x86 CPUs.
   * - _mm_loadh_pd
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with scalar loads + shuffle.
   * - _mm_loadl_epi64
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with scalar loads + shuffle.
   * - _mm_loadl_pd
     - ❌ No Wasm SIMD support. :raw-html:`<br />` Emulated with scalar loads + shuffle.
   * - _mm_loadr_pd
     - 💡 Virtual. Simd load + shuffle.
   * - _mm_loadu_pd
     - 🟡 wasm_v128_load. VM must guess type.
   * - _mm_loadu_si128
     - 🟡 wasm_v128_load. VM must guess type.
   * - _mm_loadu_si64
     - ❌ emulated with const+scalar load+replace lane
   * - _mm_loadu_si32
     - ❌ emulated with const+scalar load+replace lane
   * - _mm_loadu_si16
     - ❌ emulated with const+scalar load+replace lane
   * - _mm_madd_epi16
     - ✅ wasm_i32x4_dot_i16x8
   * - _mm_maskmoveu_si128
     - ❌ scalarized
   * - _mm_max_epi16
     - ✅ wasm_i16x8_max
   * - _mm_max_epu8
     - ✅ wasm_u8x16_max
   * - _mm_max_pd
     - TODO: migrate to wasm_f64x2_pmax
   * - _mm_max_sd
     - ⚠️ emulated with a shuffle
   * - _mm_mfence
     - ⚠️ A full barrier in multithreaded builds.
   * - _mm_min_epi16
     - ✅ wasm_i16x8_min
   * - _mm_min_epu8
     - ✅ wasm_u8x16_min
   * - _mm_min_pd
     - TODO: migrate to wasm_f64x2_pmin
   * - _mm_min_sd
     - ⚠️ emulated with a shuffle
   * - _mm_move_epi64
     - 💡 emulated with a shuffle. VM must guess type.
   * - _mm_move_sd
     - 💡 emulated with a shuffle. VM must guess type.
   * - _mm_movemask_epi8
     - ✅ wasm_i8x16_bitmask
   * - _mm_movemask_pd
     - ✅ wasm_i64x2_bitmask
   * - _mm_mul_epu32
     - ⚠️ emulated with wasm_u64x2_extmul_low_u32x4 + 2 shuffles
   * - _mm_mul_pd
     - ✅ wasm_f64x2_mul
   * - _mm_mul_sd
     - ⚠️ emulated with a shuffle
   * - _mm_mulhi_epi16
     - ⚠️ emulated with a 2x SIMD extmul+generic shuffle
   * - _mm_mulhi_epu16
     - ⚠️ emulated with a 2x SIMD extmul+generic shuffle
   * - _mm_mullo_epi16
     - ✅ wasm_i16x8_mul
   * - _mm_or_pd
     - 🟡 wasm_v128_or. VM must guess type.
   * - _mm_or_si128
     - 🟡 wasm_v128_or. VM must guess type.
   * - _mm_packs_epi16
     - ✅ wasm_i8x16_narrow_i16x8
   * - _mm_packs_epi32
     - ✅ wasm_i16x8_narrow_i32x4
   * - _mm_packus_epi16
     - ✅ wasm_u8x16_narrow_i16x8
   * - _mm_pause
     - 💭 No-op.
   * - _mm_sad_epu8
     - ⚠️ emulated with eleven SIMD instructions+const
   * - _mm_set_epi16
     - ✅ wasm_i16x8_make
   * - _mm_set_epi32
     - ✅ wasm_i32x4_make
   * - _mm_set_epi64 (_mm_set_epi64x)
     - ✅ wasm_i64x2_make
   * - _mm_set_epi8
     - ✅ wasm_i8x16_make
   * - _mm_set_pd
     - ✅ wasm_f64x2_make
   * - _mm_set_sd
     - 💡 emulated with wasm_f64x2_make
   * - _mm_set1_epi16
     - ✅ wasm_i16x8_splat
   * - _mm_set1_epi32
     - ✅ wasm_i32x4_splat
   * - _mm_set1_epi64 (_mm_set1_epi64x)
     - ✅ wasm_i64x2_splat
   * - _mm_set1_epi8
     - ✅ wasm_i8x16_splat
   * - _mm_set1_pd (_mm_set_pd1)
     - ✅ wasm_f64x2_splat
   * - _mm_setr_epi16
     - ✅ wasm_i16x8_make
   * - _mm_setr_epi32
     - ✅ wasm_i32x4_make
   * - _mm_setr_epi64
     - ✅ wasm_i64x2_make
   * - _mm_setr_epi8
     - ✅ wasm_i8x16_make
   * - _mm_setr_pd
     - ✅ wasm_f64x2_make
   * - _mm_setzero_pd
     - 💡 emulated with wasm_f64x2_const
   * - _mm_setzero_si128
     - 💡 emulated with wasm_i64x2_const
   * - _mm_shuffle_epi32
     - 💡 emulated with a general shuffle
   * - _mm_shuffle_pd
     - 💡 emulated with a general shuffle
   * - _mm_shufflehi_epi16
     - 💡 emulated with a general shuffle
   * - _mm_shufflelo_epi16
     - 💡 emulated with a general shuffle
   * - _mm_sll_epi16
     - ❌ scalarized
   * - _mm_sll_epi32
     - ❌ scalarized
   * - _mm_sll_epi64
     - ❌ scalarized
   * - _mm_slli_epi16
     - 💡 wasm_i16x8_shl :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_slli_epi32
     - 💡 wasm_i32x4_shl :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_slli_epi64
     - 💡 wasm_i64x2_shl :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_slli_si128 (_mm_bslli_si128)
     - 💡 emulated with a general shuffle
   * - _mm_sqrt_pd
     - ✅ wasm_f64x2_sqrt
   * - _mm_sqrt_sd
     - ⚠️ emulated with a shuffle
   * - _mm_sra_epi16
     - ❌ scalarized
   * - _mm_sra_epi32
     - ❌ scalarized
   * - _mm_srai_epi16
     - 💡 wasm_i16x8_shr :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_srai_epi32
     - 💡 wasm_i32x4_shr :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_srl_epi16
     - ❌ scalarized
   * - _mm_srl_epi32
     - ❌ scalarized
   * - _mm_srl_epi64
     - ❌ scalarized
   * - _mm_srli_epi16
     - 💡 wasm_u16x8_shr :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_srli_epi32
     - 💡 wasm_u32x4_shr :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_srli_epi64
     - 💡 wasm_u64x2_shr :raw-html:`<br />` ✅ if shift count is immediate constant.
   * - _mm_srli_si128 (_mm_bsrli_si128)
     - 💡 emulated with a general shuffle
   * - _mm_store_pd
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` Unaligned store on x86 CPUs.
   * - _mm_store_sd
     - 💡 emulated with scalar store
   * - _mm_store_si128
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` Unaligned store on x86 CPUs.
   * - _mm_store1_pd (_mm_store_pd1)
     - 🟡 Virtual. Emulated with shuffle. :raw-html:`<br />` Unaligned store on x86 CPUs.
   * - _mm_storeh_pd
     - ❌ shuffle + scalar stores
   * - _mm_storel_epi64
     - ❌ scalar store
   * - _mm_storel_pd
     - ❌ scalar store
   * - _mm_storer_pd
     - ❌ shuffle + scalar stores
   * - _mm_storeu_pd
     - 🟡 wasm_v128_store. VM must guess type.
   * - _mm_storeu_si128
     - 🟡 wasm_v128_store. VM must guess type.
   * - _mm_storeu_si64
     - 💡 emulated with extract lane+scalar store
   * - _mm_storeu_si32
     - 💡 emulated with extract lane+scalar store
   * - _mm_storeu_si16
     - 💡 emulated with extract lane+scalar store
   * - _mm_stream_pd
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` No cache control in Wasm SIMD.
   * - _mm_stream_si128
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` No cache control in Wasm SIMD.
   * - _mm_stream_si32
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` No cache control in Wasm SIMD.
   * - _mm_stream_si64
     - 🟡 wasm_v128_store. VM must guess type. :raw-html:`<br />` No cache control in Wasm SIMD.
   * - _mm_sub_epi16
     - ✅ wasm_i16x8_sub
   * - _mm_sub_epi32
     - ✅ wasm_i32x4_sub
   * - _mm_sub_epi64
     - ✅ wasm_i64x2_sub
   * - _mm_sub_epi8
     - ✅ wasm_i8x16_sub
   * - _mm_sub_pd
     - ✅ wasm_f64x2_sub
   * - _mm_sub_sd
     - ⚠️ emulated with a shuffle
   * - _mm_subs_epi16
     - ✅ wasm_i16x8_sub_sat
   * - _mm_subs_epi8
     - ✅ wasm_i8x16_sub_sat
   * - _mm_subs_epu16
     - ✅ wasm_u16x8_sub_sat
   * - _mm_subs_epu8
     - ✅ wasm_u8x16_sub_sat
   * - _mm_ucomieq_sd
     - ❌ scalarized
   * - _mm_ucomige_sd
     - ❌ scalarized
   * - _mm_ucomigt_sd
     - ❌ scalarized
   * - _mm_ucomile_sd
     - ❌ scalarized
   * - _mm_ucomilt_sd
     - ❌ scalarized
   * - _mm_ucomineq_sd
     - ❌ scalarized
   * - _mm_undefined_pd
     - ✅ Virtual
   * - _mm_undefined_si128
     - ✅ Virtual
   * - _mm_unpackhi_epi16
     - 💡 emulated with a shuffle
   * - _mm_unpackhi_epi32
     - 💡 emulated with a shuffle
   * - _mm_unpackhi_epi64
     - 💡 emulated with a shuffle
   * - _mm_unpackhi_epi8
     - 💡 emulated with a shuffle
   * - _mm_unpachi_pd
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_epi16
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_epi32
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_epi64
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_epi8
     - 💡 emulated with a shuffle
   * - _mm_unpacklo_pd
     - 💡 emulated with a shuffle
   * - _mm_xor_pd
     - 🟡 wasm_v128_xor. VM must guess type.
   * - _mm_xor_si128
     - 🟡 wasm_v128_xor. VM must guess type.

⚫ The following extensions that SSE2 instruction set brought to 64-bit wide MMX registers are not available:
 - _mm_add_si64, _mm_movepi64_pi64, _mm_movpi64_epi64, _mm_mul_su32, _mm_sub_si64, _mm_cvtpd_pi32, _mm_cvtpi32_pd, _mm_cvttpd_pi32

Any code referencing these intrinsics will not compile.

The following table highlights the availability and expected performance of different SSE3 intrinsics. Refer to `Intel Intrinsics Guide on SSE3 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE3>`_.

.. list-table:: x86 SSE3 intrinsics available via #include <pmmintrin.h> and -msse3
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_lddqu_si128
     - ✅ wasm_v128_load.
   * - _mm_addsub_ps
     - ⚠️ emulated with a SIMD add+mul+const
   * - _mm_hadd_ps
     - ⚠️ emulated with a SIMD add+two shuffles
   * - _mm_hsub_ps
     - ⚠️ emulated with a SIMD sub+two shuffles
   * - _mm_movehdup_ps
     - 💡 emulated with a general shuffle
   * - _mm_moveldup_ps
     - 💡 emulated with a general shuffle
   * - _mm_addsub_pd
     - ⚠️ emulated with a SIMD add+mul+const
   * - _mm_hadd_pd
     - ⚠️ emulated with a SIMD add+two shuffles
   * - _mm_hsub_pd
     - ⚠️ emulated with a SIMD add+two shuffles
   * - _mm_loaddup_pd
     - 🟡 Virtual. wasm_v64x2_load_splat, VM must guess type.
   * - _mm_movedup_pd
     - 💡 emulated with a general shuffle
   * - _MM_GET_DENORMALS_ZERO_MODE
     - ✅ Always returns _MM_DENORMALS_ZERO_ON. I.e. denormals are available.
   * - _MM_SET_DENORMALS_ZERO_MODE
     - ⚫ Not available. Fixed to _MM_DENORMALS_ZERO_ON.
   * - _mm_monitor
     - ⚫ Not available.
   * - _mm_mwait
     - ⚫ Not available.

The following table highlights the availability and expected performance of different SSSE3 intrinsics. Refer to `Intel Intrinsics Guide on SSSE3 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSSE3>`_.

.. list-table:: x86 SSSE3 intrinsics available via #include <tmmintrin.h> and -mssse3
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_abs_epi8
     - ✅ wasm_i8x16_abs
   * - _mm_abs_epi16
     - ✅ wasm_i16x8_abs
   * - _mm_abs_epi32
     - ✅ wasm_i32x4_abs
   * - _mm_alignr_epi8
     - ⚠️ emulated with a SIMD or+two shifts
   * - _mm_hadd_epi16
     - ⚠️ emulated with a SIMD add+two shuffles
   * - _mm_hadd_epi32
     - ⚠️ emulated with a SIMD add+two shuffles
   * - _mm_hadds_epi16
     - ⚠️ emulated with a SIMD adds+two shuffles
   * - _mm_hsub_epi16
     - ⚠️ emulated with a SIMD sub+two shuffles
   * - _mm_hsub_epi32
     - ⚠️ emulated with a SIMD sub+two shuffles
   * - _mm_hsubs_epi16
     - ⚠️ emulated with a SIMD subs+two shuffles
   * - _mm_maddubs_epi16
     - ⚠️ emulated with SIMD saturated add+four shifts+two muls+and+const
   * - _mm_mulhrs_epi16
     - ⚠️ emulated with SIMD four widen+two muls+four adds+complex shuffle+const
   * - _mm_shuffle_epi8
     - ⚠️ emulated with a SIMD swizzle+and+const
   * - _mm_sign_epi8
     - ⚠️ emulated with SIMD two cmp+two logical+add
   * - _mm_sign_epi16
     - ⚠️ emulated with SIMD two cmp+two logical+add
   * - _mm_sign_epi32
     - ⚠️ emulated with SIMD two cmp+two logical+add

⚫ The SSSE3 functions that deal with 64-bit wide MMX registers are not available:
 -  _mm_abs_pi8, _mm_abs_pi16, _mm_abs_pi32, _mm_alignr_pi8, _mm_hadd_pi16, _mm_hadd_pi32, _mm_hadds_pi16, _mm_hsub_pi16, _mm_hsub_pi32, _mm_hsubs_pi16, _mm_maddubs_pi16, _mm_mulhrs_pi16, _mm_shuffle_pi8, _mm_sign_pi8, _mm_sign_pi16 and _mm_sign_pi32

Any code referencing these intrinsics will not compile.

The following table highlights the availability and expected performance of different SSE4.1 intrinsics. Refer to `Intel Intrinsics Guide on SSE4.1 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE4_1>`_.

.. list-table:: x86 SSE4.1 intrinsics available via #include <smmintrin.h> and -msse4.1
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_blend_epi16
     - 💡 emulated with a general shuffle
   * - _mm_blend_pd
     - 💡 emulated with a general shuffle
   * - _mm_blend_ps
     - 💡 emulated with a general shuffle
   * - _mm_blendv_epi8
     - ⚠️ emulated with a SIMD shr+and+andnot+or
   * - _mm_blendv_pd
     - ⚠️ emulated with a SIMD shr+and+andnot+or
   * - _mm_blendv_ps
     - ⚠️ emulated with a SIMD shr+and+andnot+or
   * - _mm_ceil_pd
     - ✅ wasm_f64x2_ceil
   * - _mm_ceil_ps
     - ✅ wasm_f32x4_ceil
   * - _mm_ceil_sd
     - ⚠️ emulated with a shuffle
   * - _mm_ceil_ss
     - ⚠️ emulated with a shuffle
   * - _mm_cmpeq_epi64
     - ⚠️ emulated with a SIMD cmp+and+shuffle
   * - _mm_cvtepi16_epi32
     - ✅ wasm_i32x4_widen_low_i16x8
   * - _mm_cvtepi16_epi64
     - ⚠️ emulated with a SIMD widen+const+cmp+shuffle
   * - _mm_cvtepi32_epi64
     - ⚠️ emulated with SIMD const+cmp+shuffle
   * - _mm_cvtepi8_epi16
     - ✅ wasm_i16x8_widen_low_i8x16
   * - _mm_cvtepi8_epi32
     - ⚠️ emulated with two SIMD widens
   * - _mm_cvtepi8_epi64
     - ⚠️ emulated with two SIMD widens+const+cmp+shuffle
   * - _mm_cvtepu16_epi32
     - ✅ wasm_u32x4_extend_low_u16x8
   * - _mm_cvtepu16_epi64
     - ⚠️ emulated with SIMD const+two shuffles
   * - _mm_cvtepu32_epi64
     - ⚠️ emulated with SIMD const+shuffle
   * - _mm_cvtepu8_epi16
     - ✅ wasm_u16x8_extend_low_u8x16
   * - _mm_cvtepu8_epi32
     - ⚠️ emulated with two SIMD widens
   * - _mm_cvtepu8_epi64
     - ⚠️ emulated with SIMD const+three shuffles
   * - _mm_dp_pd
     - ⚠️ emulated with SIMD mul+add+setzero+2xblend
   * - _mm_dp_ps
     - ⚠️ emulated with SIMD mul+add+setzero+2xblend
   * - _mm_extract_epi32
     - ✅ wasm_i32x4_extract_lane
   * - _mm_extract_epi64
     - ✅ wasm_i64x2_extract_lane
   * - _mm_extract_epi8
     - ✅ wasm_u8x16_extract_lane
   * - _mm_extract_ps
     - ✅ wasm_i32x4_extract_lane
   * - _mm_floor_pd
     - ✅ wasm_f64x2_floor
   * - _mm_floor_ps
     - ✅ wasm_f32x4_floor
   * - _mm_floor_sd
     - ⚠️ emulated with a shuffle
   * - _mm_floor_ss
     - ⚠️ emulated with a shuffle
   * - _mm_insert_epi32
     - ✅ wasm_i32x4_replace_lane
   * - _mm_insert_epi64
     - ✅ wasm_i64x2_replace_lane
   * - _mm_insert_epi8
     - ✅ wasm_i8x16_replace_lane
   * - _mm_insert_ps
     - ⚠️ emulated with generic non-SIMD-mapping shuffles
   * - _mm_max_epi32
     - ✅ wasm_i32x4_max
   * - _mm_max_epi8
     - ✅ wasm_i8x16_max
   * - _mm_max_epu16
     - ✅ wasm_u16x8_max
   * - _mm_max_epu32
     - ✅ wasm_u32x4_max
   * - _mm_min_epi32
     - ✅ wasm_i32x4_min
   * - _mm_min_epi8
     - ✅ wasm_i8x16_min
   * - _mm_min_epu16
     - ✅ wasm_u16x8_min
   * - _mm_min_epu32
     - ✅ wasm_u32x4_min
   * - _mm_minpos_epu16
     - 💣 scalarized
   * - _mm_mpsadbw_epu8
     - 💣 scalarized
   * - _mm_mul_epi32
     - ⚠️ emulated with wasm_i64x2_extmul_low_i32x4 + 2 shuffles
   * - _mm_mullo_epi32
     - ✅ wasm_i32x4_mul
   * - _mm_packus_epi32
     - ✅ wasm_u16x8_narrow_i32x4
   * - _mm_round_pd
     - ✅ wasm_f64x2_ceil/wasm_f64x2_floor/wasm_f64x2_nearest/wasm_f64x2_trunc
   * - _mm_round_ps
     - ✅ wasm_f32x4_ceil/wasm_f32x4_floor/wasm_f32x4_nearest/wasm_f32x4_trunc
   * - _mm_round_sd
     - ⚠️ emulated with a shuffle
   * - _mm_round_ss
     - ⚠️ emulated with a shuffle
   * - _mm_stream_load_si128
     - 🟡 wasm_v128_load. VM must guess type. :raw-html:`<br />` Unaligned load on x86 CPUs.
   * - _mm_test_all_ones
     - ❌ scalarized
   * - _mm_test_all_zeros
     - ❌ scalarized
   * - _mm_test_mix_ones_zeros
     - ❌ scalarized
   * - _mm_testc_si128
     - ❌ scalarized
   * - _mm_testnzc_si128
     - ❌ scalarized
   * - _mm_testz_si128
     - ❌ scalarized

The following table highlights the availability and expected performance of different SSE4.2 intrinsics. Refer to `Intel Intrinsics Guide on SSE4.2 <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE4_2>`_.

.. list-table:: x86 SSE4.2 intrinsics available via #include <nmmintrin.h> and -msse4.2
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_cmpgt_epi64
     - ✅ wasm_i64x2_gt

⚫ The SSE4.2 functions that deal with string comparisons and CRC calculations are not available:
 - _mm_cmpestra, _mm_cmpestrc, _mm_cmpestri, _mm_cmpestrm, _mm_cmpestro, _mm_cmpestrs, _mm_cmpestrz, _mm_cmpistra, _mm_cmpistrc, _mm_cmpistri, _mm_cmpistrm, _mm_cmpistro, _mm_cmpistrs, _mm_cmpistrz, _mm_crc32_u16, _mm_crc32_u32, _mm_crc32_u64, _mm_crc32_u8

Any code referencing these intrinsics will not compile.

The following table highlights the availability and expected performance of different AVX intrinsics. Refer to `Intel Intrinsics Guide on AVX <https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=AVX>`_.

.. list-table:: x86 AVX intrinsics available via #include <immintrin.h> and -mavx
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_broadcast_ss
     - ✅ wasm_v32x4_load_splat
   * - _mm_cmp_pd
     - ⚠️ emulated with 1-2 SIMD cmp+and/or
   * - _mm_cmp_ps
     - ⚠️ emulated with 1-2 SIMD cmp+and/or
   * - _mm_cmp_sd
     - ⚠️ emulated with 1-2 SIMD cmp+and/or+move
   * - _mm_cmp_ss
     - ⚠️ emulated with 1-2 SIMD cmp+and/or+move
   * - _mm_maskload_pd
     - ⚠️ emulated with SIMD load+shift+and
   * - _mm_maskload_ps
     - ⚠️ emulated with SIMD load+shift+and
   * - _mm_maskstore_pd
     - ❌ scalarized
   * - _mm_maskstore_ps
     - ❌ scalarized
   * - _mm_permute_pd
     - 💡 emulated with a general shuffle
   * - _mm_permute_ps
     - 💡 emulated with a general shuffle
   * - _mm_permutevar_pd
     - 💣 scalarized
   * - _mm_permutevar_ps
     - 💣 scalarized
   * - _mm_testc_pd
     - 💣 emulated with complex SIMD+scalar sequence
   * - _mm_testc_ps
     - 💣 emulated with complex SIMD+scalar sequence
   * - _mm_testnzc_pd
     - 💣 emulated with complex SIMD+scalar sequence
   * - _mm_testnzc_ps
     - 💣 emulated with complex SIMD+scalar sequence
   * - _mm_testz_pd
     - 💣 emulated with complex SIMD+scalar sequence
   * - _mm_testz_ps
     - 💣 emulated with complex SIMD+scalar sequence

Only the 128-bit wide instructions from AVX instruction set are listed. The 256-bit wide AVX instructions are emulated by two 128-bit wide instructions.


====================================================== 
Compiling SIMD code targeting ARM NEON instruction set
======================================================

Emscripten supports compiling existing codebases that use ARM NEON by
passing the `-mfpu=neon` directive to the compiler, and including the
header `<arm_neon.h>`.

In terms of performance, it is very important to note that only
instructions which operate on 128-bit wide vectors are supported
cleanly. This means that nearly any instruction which is not of a "q"
variant (i.e. "vaddq" as opposed to "vadd") will be scalarized.

These are pulled from `SIMDe repository on GitHub
<https://github.com/simd-everywhere/simde>`_. To update emscripten
with the latest SIMDe version, run `tools/simde_update.py`.

The following table highlights the availability of various 128-bit
wide intrinsics.

Similarly to above, the following legend is used:
 - ✅ Wasm SIMD has a native opcode that matches the NEON instruction, should yield native performance
 - 💡 while the Wasm SIMD spec does not provide a proper performance guarantee, given a suitably smart enough compiler and a runtime VM path, this intrinsic should be able to generate the identical native NEON instruction.
 - ⚠️ the underlying NEON instruction is not available, but it is emulated via at most few other Wasm SIMD instructions, causing a small penalty.
 - ❌ the underlying NEON instruction is not exposed by the Wasm SIMD specification, so it must be emulated via a slow path, e.g. a sequence of several slower SIMD instructions, or a scalar implementation.
 - ⚫ the given NEON intrinsic is not available. Referencing the intrinsic will cause a compiler error.

For detailed information on each intrinsic function, refer to `NEON Intrinsics Reference
<https://developer.arm.com/architectures/instruction-sets/simd-isas/neon/intrinsics>`_.

For the latest NEON intrinsics implementation status, refer to the `SIMDe implementation
status <https://github.com/simd-everywhere/implementation-status/blob/main/neon.md>`_.

.. list-table:: NEON Intrinsics
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - Wasm SIMD Support
   * - vaba
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vabaq
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vabal
     - ⚫ Not implemented, will trigger compiler error
   * - vabd
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vabdq
     - ✅ native
   * - vabdl
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vabs
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vabq
     - ✅ native
   * - vadd
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vaddq_s & vaddq_f
     - ✅ native
   * - vaddhn
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vaddl
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vaddlv
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vaddv
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vaddw
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vand
     - ✅ native
   * - vbcaxq
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vbic
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vbiq
     - ✅ native
   * - vbsl
     - ✅ native
   * - vcage
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vcagt
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vceq
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vceqz
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vcge
     - ✅ native
   * - vcgez
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vcgt
     - ✅ native
   * - vcgtz
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vcle
     - ✅ native
   * - vclez
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vcls
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vclt
     - ✅ native
   * - vcltz 
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vcmla, vcmla_rot90, cmla_rot180, cmla_rot270
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vcmlq
     - ✅ native
   * - vcnt
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vclz
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vcombine 
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vcreate
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vdot
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vdot_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vdup
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vdup_n
     - ✅ native
   * - veor
     - ✅ native
   * - vext
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vfma, vfma_lane, vfma_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vget_lane
     - ✅ native
   * - vhadd
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vhsub
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vld1
     - ✅ native
   * - vld2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vld3
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vld4
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vld4_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmax
     - ✅ native
   * - vmaxv
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmin
     - ✅ native
   * - vminv
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmla 
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vmlal
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlal_high_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlal_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmls
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmls_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlsl
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlsl_high
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlsl_high_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmlsl_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmovl
     - ✅ native
   * - vmul
     - ✅ native
   * - vmul_n 
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vmull 
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vmull_n
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vmull_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmull_high
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vmvn
     - ✅ native
   * - vneg
     - ✅ native
   * - vorn
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vorr
     - ✅ native
   * - vpadal
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vpadd
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vpaddl 
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vpmax
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vpmin
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vpminnm
     - ⚫ Not implemented, will trigger compiler error
   * - vqabs
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqabsb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqadd 
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vqaddb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqdmulh  
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqdmulh_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqneg
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqnegb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqrdmulh
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqrdmulh_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqshl
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqshlb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqshrn_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqshrun_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqsub
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqsubb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqtbl1
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vqtbl2
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vqtbl3
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vqtbl4
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vqtbx1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqtbx2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqtbx3
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vqtbx4
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrbit
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vrecpe
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrecps
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vreinterpret
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vrev16
     - ✅ native
   * - vrev32
     - ✅ native
   * - vrev64
     - ✅ native
   * - vrhadd
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vrsh_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrshn_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrsqrte
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrsqrts
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrshl
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrshr_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vrsra_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vset_lane
     - ✅ native
   * - vshl
     - scalaried
   * - vshl_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vshll_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vshr_n
     - ⚠️ does not have direct implementation, but is emulated using fast neon instructions
   * - vshrn_n
     - ⚠️ does not have direct implementation, but is emulated using fast neon instructions
   * - vsqadd
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vsra_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vsri_n
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vst1
     - ✅ native
   * - vst1_lane
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vst2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vst2_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vst3
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vst3_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vst4
     - 💡 Depends on a smart enough compiler, but should be near native
   * - vst4_lane
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vsub
     - ✅ native
   * - vsubl
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vsubl_high
     - ⚠️ Does not have direct implementation, but is emulated using fast NEON instructions
   * - vsubn
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vsubw
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbl1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbl2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbl3
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbl4
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbx1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbx2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbx3
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtbx4
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtrn
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtrn1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtrn2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vtst
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vuqadd
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vuqaddb
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vuzp
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vuzp1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vuzp2
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vxar
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vzip
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vzip1
     - ❌ Will be emulated with slow instructions, or scalarized
   * - vzip2
     - ❌ Will be emulated with slow instructions, or scalarized
