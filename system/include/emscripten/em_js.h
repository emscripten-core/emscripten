/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#ifndef __em_js_h__
#define __em_js_h__

#ifdef __cplusplus
#define _EM_JS_CPP_BEGIN extern "C" {
#define _EM_JS_CPP_END   }
#else // __cplusplus
#define _EM_JS_CPP_BEGIN
#define _EM_JS_CPP_END
#endif // __cplusplus

// EM_JS declares JS functions in C code.
// Example uses can be found in tests/core/test_em_js.cpp

// Implementation details:

// The EM_JS macro is specified as:
//   EM_JS(return type, function name, (arguments), {body})
// but the macro uses __VA_ARGS__ instead of a fourth argument. This is so that
// function bodies containing commas are seamlessly handled.

// EM_JS declares the JS function with a C function prototype, which becomes a
// function import in asm.js/wasm. It also declares an __em_js__-prefixed
// function, which we can use to pass information to the Emscripten compiler
// that survives going through LLVM.
// Example:
//   EM_JS(int, foo, (int x, int y), { return 2 * x + y; })
// would get translated into:
//   int foo(int x, int y);
//   __attribute__((used, visibility("default")))
//   const char* __em_js__foo() {
//     return "(int x, int y)<::>{ return 2 * x + y; }";
//   }
// We pack the arguments and function body into a constant string so it's
// readable from asm.js/wasm post-processing.
// Later we see a function called __em_js__foo, meaning we need to create a JS
// function:
//   function foo(x, y) { return 2 * x + y; }
// We use <::> to separate the arguments from the function body because it isn't
// valid anywhere in a C function declaration.

// Generated __em_js__-prefixed functions are read by either the JSBackend (for
// asm.js) or by Binaryen, and the string data is extracted into the Emscripten
// metadata dictionary under the "emJsFuncs" key. emJsFuncs itself is a
// dictionary where the keys are function names (not prefixed with __em_js__),
// and the values are the <::>-including description strings.

// emJsFuncs metadata is read in emscripten.py's create_em_js, which creates an
// array of JS function strings to be included in the JS output.

#define EM_JS(ret, name, params, ...)          \
  _EM_JS_CPP_BEGIN                             \
  extern ret name params;                      \
  __attribute__((used, visibility("default"))) \
  const char* __em_js__##name() {              \
    return #params "<::>" #__VA_ARGS__;        \
  }                                            \
  _EM_JS_CPP_END

#endif // __em_js_h__
