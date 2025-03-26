/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Legacy proxying functions.  See proxying.h for the new proxying system.
 */

#pragma once

#include <stdarg.h>

#include <emscripten/html5.h>

#ifdef __cplusplus
extern "C" {
#endif

#define emscripten_main_browser_thread_id() emscripten_main_runtime_thread_id()
#pragma clang deprecated(emscripten_main_browser_thread_id, "use emscripten_main_runtime_thread_id instead")

typedef struct em_queued_call em_queued_call;

// Encode function signatures into a single uint32_t integer.
// N.B. This encoding scheme is internal to the implementation, and can change
// in the future. Do not depend on the exact numbers in this scheme.
#define EM_FUNC_SIGNATURE unsigned int

// Proxied JS function can support a few more arguments than proxied C/C++
// functions, because the dispatch is variadic and signature independent.
#define EM_QUEUED_JS_CALL_MAX_ARGS 20

// The encoding scheme is as follows:
// - highest three bits identify the type of the return value
#define EM_FUNC_SIG_RETURN_VALUE_MASK (0x7U << 29)

#define EM_FUNC_SIG_RETURN_VALUE_V   0
#define EM_FUNC_SIG_RETURN_VALUE_I   (0x1U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_J   (0x2U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_F   (0x3U << 29)
#define EM_FUNC_SIG_RETURN_VALUE_D   (0x4U << 29)
#if __wasm64__
#define EM_FUNC_SIG_RETURN_VALUE_P   EM_FUNC_SIG_RETURN_VALUE_J
#else
#define EM_FUNC_SIG_RETURN_VALUE_P   EM_FUNC_SIG_RETURN_VALUE_I
#endif

// - next highest four bits specify the number of input parameters to the
//   function (allowed values are 0-12, inclusively)
#define EM_FUNC_SIG_NUM_PARAMETERS_SHIFT 25
#define EM_FUNC_SIG_NUM_PARAMETERS_MASK (0xFU << EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)
#define EM_FUNC_SIG_WITH_N_PARAMETERS(x) (((EM_FUNC_SIGNATURE)(x)) << EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)

// - starting from the lowest bits upwards, each pair of two subsequent bits
//   specifies the type of an input parameter.
//   That is, bits 1:0 encode the type of the first input, bits 3:2 encode the
//   type of the second input, and so on.
#define EM_FUNC_SIG_ARGUMENTS_TYPE_MASK (~(EM_FUNC_SIG_RETURN_VALUE_MASK | EM_FUNC_SIG_NUM_PARAMETERS_MASK))
#define EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_MASK 0x3U
#define EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT 2

#define EM_FUNC_SIG_PARAM_I   0
#define EM_FUNC_SIG_PARAM_J   0x1U
#define EM_FUNC_SIG_PARAM_F   0x2U
#define EM_FUNC_SIG_PARAM_D   0x3U
#if __wasm64__
#define EM_FUNC_SIG_PARAM_P   EM_FUNC_SIG_PARAM_J
#else
#define EM_FUNC_SIG_PARAM_P   EM_FUNC_SIG_PARAM_I
#endif
#define EM_FUNC_SIG_SET_PARAM(i, type) ((EM_FUNC_SIGNATURE)(type) << (EM_FUNC_SIG_ARGUMENT_TYPE_SIZE_SHIFT*i))

// Extra types used in WebGL glGet*() calls (not used in proxying)
#define EM_FUNC_SIG_PARAM_B   0x4U
#define EM_FUNC_SIG_PARAM_F2I 0x5U

// In total, the above encoding scheme gives the following 32-bit structure for
// the proxied function signatures (highest -> lowest bit order):
// RRRiiiiSbbaa99887766554433221100
// where RRR is return type
// iiii is the number of inputs
// S denotes a special function (internal proxying mechanism for functions
// related to built-in threading APIs, like thread creation itself)
// 00-bb encode the type of up to 12 function parameters

#define EM_FUNC_SIG_V     (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_D     (EM_FUNC_SIG_RETURN_VALUE_D | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_VI    (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VF    (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_FI    (EM_FUNC_SIG_RETURN_VALUE_F | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_DI    (EM_FUNC_SIG_RETURN_VALUE_D | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VII   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIF   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFF   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIII  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIFF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFFF  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIFI (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VFFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_IIFFF (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIFFFF (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_F) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_F))
#define EM_FUNC_SIG_VIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(6) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(7) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(8) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(9) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(10) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(9, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(11) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(9, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(10, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_I     (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(0))
#define EM_FUNC_SIG_II    (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_III   (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIII  (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIPP  (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_IIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(5) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(6) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(7) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(8) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_IIIIIIIIII (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(9) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_I))

#define EM_FUNC_SIG_IP    (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_PI    (EM_FUNC_SIG_RETURN_VALUE_P | EM_FUNC_SIG_WITH_N_PARAMETERS(1) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_PPP   (EM_FUNC_SIG_RETURN_VALUE_P | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_PII   (EM_FUNC_SIG_RETURN_VALUE_P | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_PIIII (EM_FUNC_SIG_RETURN_VALUE_P | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIP  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIPI  (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(3) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIP   (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_IIP   (EM_FUNC_SIG_RETURN_VALUE_I | EM_FUNC_SIG_WITH_N_PARAMETERS(2) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIIPP (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIPPI (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_P) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I))
#define EM_FUNC_SIG_VIIIP (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(4) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIIIIIP (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(6) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIIIIIIP (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(7) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_P))
#define EM_FUNC_SIG_VIIIIIIIIP (EM_FUNC_SIG_RETURN_VALUE_V | EM_FUNC_SIG_WITH_N_PARAMETERS(9) | EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(1, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(2, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(3, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(4, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(5, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(6, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(7, EM_FUNC_SIG_PARAM_I) | EM_FUNC_SIG_SET_PARAM(8, EM_FUNC_SIG_PARAM_P))

#define EM_FUNC_SIG_NUM_FUNC_ARGUMENTS(x) ((((EM_FUNC_SIGNATURE)x) & EM_FUNC_SIG_NUM_PARAMETERS_MASK) >> EM_FUNC_SIG_NUM_PARAMETERS_SHIFT)

// There are some built-in special proxied functions, that embed the signatures
// inside the above encoding scheme
#define EM_FUNC_SIG_SPECIAL_INTERNAL (1 << 24)
#define EM_PROXIED_FUNC_SPECIAL(x) (EM_FUNC_SIG_SPECIAL_INTERNAL | ((x) << 20))

// Runs the given function synchronously on the main Emscripten runtime thread.
// If this thread is the main thread, the operation is immediately performed,
// and the result is returned.
// If the current thread is not the main Emscripten runtime thread (but a
// pthread), the function
// will be proxied to be called by the main thread.
//  - Calling emscripten_sync_* functions requires that the application was
//    compiled with pthreads support enabled (-pthread) and that the
//    browser supports SharedArrayBuffer specification.
int emscripten_sync_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr __attribute__((nonnull)), ...);

// The 'async' variant of the run_in_main_thread functions are otherwise the
// same as the synchronous ones, except that the operation is performed in a
// fire and forget manner. The call is placed to the command queue of the main
// Emscripten runtime thread, but its completion is not waited for. As a result,
// if the function did have a return value, the return value is not received.
//  - Note that multiple asynchronous commands from a single pthread/Worker are
//    guaranteed to be executed on the main thread in the program order they
//    were called in.
void emscripten_async_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr __attribute__((nonnull)), ...);

// The 'async_waitable' variant of the run_in_main_runtime_thread functions run
// like the 'async' variants, except that while the operation starts off
// asynchronously, the result is then later waited upon to receive the return
// value.
//  - The object returned by this function call is dynamically allocated, and
//    should be freed up via a call to emscripten_async_waitable_close() after
//    the wait has been performed.
em_queued_call *emscripten_async_waitable_run_in_main_runtime_thread_(EM_FUNC_SIGNATURE sig, void *func_ptr __attribute__((nonnull)), ...);

// Since we can't validate the function pointer type, allow implicit casting of
// functions to void* without complaining.
#define emscripten_sync_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_sync_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)

#ifdef __wasm64__
// For wasm64 we need to special handling of pointer (P) return types since
// int and pointer have different widths
void* emscripten_sync_run_in_main_runtime_thread_ptr_(EM_FUNC_SIGNATURE sig, void *func_ptr __attribute__((nonnull)), ...);
#define emscripten_sync_run_in_main_runtime_thread_ptr(sig, func_ptr, ...) emscripten_sync_run_in_main_runtime_thread_ptr_((sig), (void*)(func_ptr),##__VA_ARGS__)
#else
#define emscripten_sync_run_in_main_runtime_thread_ptr emscripten_sync_run_in_main_runtime_thread
#endif

#define emscripten_async_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)
#define emscripten_async_waitable_run_in_main_runtime_thread(sig, func_ptr, ...) emscripten_async_waitable_run_in_main_runtime_thread_((sig), (void*)(func_ptr),##__VA_ARGS__)

EMSCRIPTEN_RESULT emscripten_wait_for_call_v(em_queued_call *call __attribute__((nonnull)), double timeoutMSecs);
EMSCRIPTEN_RESULT emscripten_wait_for_call_i(em_queued_call *call __attribute__((nonnull)), double timeoutMSecs, int *outResult);

void emscripten_async_waitable_close(em_queued_call *call __attribute__((nonnull)));

// Runs the given function on the specified thread. If we are currently on
// that target thread then we just execute the call synchronously; otherwise it
// is queued on that thread to execute asynchronously.
// Returns 1 if it executed the code (i.e., it was on the target thread), and 0
// otherwise.
int emscripten_dispatch_to_thread_args(pthread_t target_thread,
                                       EM_FUNC_SIGNATURE sig,
                                       void* func_ptr __attribute__((nonnull)),
                                       void* satellite,
                                       va_list args);
int emscripten_dispatch_to_thread_(pthread_t target_thread,
                                   EM_FUNC_SIGNATURE sig,
                                   void* func_ptr __attribute__((nonnull)),
                                   void* satellite,
                                   ...);
#define emscripten_dispatch_to_thread(                                         \
  target_thread, sig, func_ptr, satellite, ...)                                \
  emscripten_dispatch_to_thread_(                                              \
    (target_thread), (sig), (void*)(func_ptr), (satellite), ##__VA_ARGS__)

// Similar to emscripten_dispatch_to_thread, but always runs the
// function asynchronously, even if on the same thread. This is less efficient
// but may be simpler to reason about in some cases.
int emscripten_dispatch_to_thread_async_args(pthread_t target_thread,
                                             EM_FUNC_SIGNATURE sig,
                                             void* func_ptr __attribute__((nonnull)),
                                             void* satellite,
                                             va_list args);
int emscripten_dispatch_to_thread_async_(pthread_t target_thread,
                                         EM_FUNC_SIGNATURE sig,
                                         void* func_ptr __attribute__((nonnull)),
                                         void* satellite,
                                         ...);
#define emscripten_dispatch_to_thread_async(                                   \
  target_thread, sig, func_ptr, satellite, ...)                                \
  emscripten_dispatch_to_thread_async_(                                        \
    (target_thread), (sig), (void*)(func_ptr), (satellite), ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
