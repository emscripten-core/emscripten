/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <emscripten/em_macros.h>

// EM_JS declares JS functions in C code.
// Example uses can be found in test/core/test_em_js.cpp

// Implementation details:

// The EM_JS macro is specified as:
//   EM_JS(return type, function name, (arguments), {body})
// but the macro uses __VA_ARGS__ instead of a fourth argument. This is so that
// function bodies containing commas are seamlessly handled.

// EM_JS declares the JS function with a C function prototype, which becomes a
// function import in wasm. It also declares an __em_js__-prefixed string constant,
// which we can use to pass information to the Emscripten compiler that survives
// going through LLVM.
// Example:
//
//   EM_JS(int, foo, (int x, int y), { return 2 * x + y; })
//
// would get translated into:
//
//   __attribute__((import_name("foo"))) int foo(int x, int y);
//
//   __attribute__((used)) static void* __em_js_ref_foo = (void*)&foo;
//
//   __attribute__((used, visibility("default")))
//   char __em_js__foo[] = "(int x, int y)<::>{ return 2 * x + y; }";
//
// We pack the arguments and function body into a constant string so it's
// readable during wasm post-processing.
// Later we see an export called __em_js__foo, meaning we need to create a JS
// function:
//   function foo(x, y) { return 2 * x + y; }
// We use <::> to separate the arguments from the function body because it isn't
// valid anywhere in a C function declaration.

// The __em_js_ref_foo pointer simply exists in order to force a reference to
// `foo` to exist in the object file, even if there are no other local uses.
// This means the linker will always use the import_name attribute for this
// function even if it is not locally used.

// Generated __em_js__-prefixed symbols are read by binaryen, and the string
// data is extracted into the Emscripten metadata dictionary under the
// "emJsFuncs" key. emJsFuncs itself is a dictionary where the keys are function
// names (not prefixed with __em_js__), and the values are the <::>-including
// description strings.

// emJsFuncs metadata is read in emscripten.py's create_em_js, which creates an
// array of JS function strings to be included in the JS output.

#define _EM_JS(ret, c_name, js_name, params, code)                             \
  _EM_BEGIN_CDECL                                                              \
  ret c_name params EM_IMPORT(js_name);                                        \
  __attribute__((visibility("hidden")))                                        \
  void* __em_js_ref_##c_name = (void*)&c_name;                                 \
  EMSCRIPTEN_KEEPALIVE                                                         \
  __attribute__((section("em_js"), aligned(1))) char __em_js__##js_name[] =    \
    #params "<::>" code;                                                       \
  _EM_END_CDECL

#define EM_JS(ret, name, params, ...) _EM_JS(ret, name, name, params, #__VA_ARGS__)

#define EM_ASYNC_JS(ret, name, params, ...) _EM_JS(ret, name, __asyncjs__##name, params,          \
  "{ return Asyncify.handleAsync(async () => " #__VA_ARGS__ "); }")
