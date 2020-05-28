.. Porting SIMD code:

.. role:: raw-html(raw)
    :format: html

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

When porting native SIMD code, it should be noted that because of portability concerns, the WebAssembly SIMD specification does not expose the full native instruction sets. In particular the following changes exist:

 - Emscripten does not support x86 or any other native inline SIMD assembly or building .s assembly files, so all code should be written to use SIMD intrinsic functions or compiler vector extensions.

 - WebAssembly SIMD does not have control over managing floating point rounding modes or handling denormals.

 - Cache line prefetch instructions are not available, and calls to these functions will compile, but are treated as no-ops.

 - Asymmetric memory fence operations are not available, but will be implemented as fully synchronous memory fences when SharedArrayBuffer is enabled (-s USE_PTHREADS=1) or as no-ops when multithreading is not enabled (default, -s USE_PTHREADS=0).

SIMD-related bug reports are tracked in the `Emscripten bug tracker with the label SIMD <https://github.com/emscripten-core/emscripten/issues?q=is%3Aopen+is%3Aissue+label%3ASIMD>`_.

=====================================================
Compiling SIMD code targeting x86 SSE instruction set
=====================================================

Emscripten supports compiling existing x86 SSE utilizing codebases by passing the `-msse` directive to the compiler, and including the header `<xmmintrin.h>`.

Currently only the SSE1 instruction set is supported.

The following table highlights the performance landscape that can be expected from the different SSE1 instrinsics. Even if you are directly targeting the native Wasm SIMD opcodes via wasm_simd128.h header, this table can be useful for understanding the performance limitations that the Wasm SIMD specification has when running on x86 hardware.

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

.. list-table:: x86 SSE intrinsics available via #include <xmmintrin.h>
   :widths: 20 30
   :header-rows: 1

   * - Intrinsic name
     - WebAssembly SIMD support
   * - _mm_set_ps
     - ✅ wasm_f32x4_make
   * - _mm_setr_ps
     - ✅ wasm_f32x4_make
   * - _mm_set_ss
     - ⚠️ emulated with wasm_f32x4_make
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
     - 🟡 wasm_v128_store. VM must guess type.
   * - _mm_prefetch
     - 💭 No-op.
   * - _mm_sfence
     - ⚠️ A full barrier in multithreaded builds.
   * - _mm_shuffle_ps
     - 🟡 wasm_v32x4_shuffle. VM must guess type.
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
     - 💣 No Wasm SIMD support. Emulated in scalar. `simd/#131 <https://github.com/WebAssembly/simd/issues/131>`_
   * - _mm_move_ss
     - 💡 emulated with a shuffle
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