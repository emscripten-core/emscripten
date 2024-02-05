/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

#ifdef __wasm__
#define EM_IMPORT(NAME) __attribute__((import_module("env"), import_name(#NAME)))
#else
#define EM_IMPORT(NAME)
#endif

#ifdef __cplusplus
#define _EM_BEGIN_CDECL extern "C" {
#define _EM_END_CDECL   }
#else // __cplusplus
#define _EM_BEGIN_CDECL
#define _EM_END_CDECL
#endif // __cplusplus

/*
 * EM_JS_DEPS: Use this macro to declare indirect dependencies on JS symbols.
 * The first argument is just unique name for the set of dependencies.  The
 * second argument is a C string that lists JS library symbols in the same way
 * they would be specified in the DEFAULT_LIBRARY_FUNCS_TO_INCLUDE command line
 * setting.
 *
 * For example, if your code contains an EM_ASM or EM_JS block that make use of
 * the allocate and stackSave JS library functions then you might write this in
 * your library source code:
 *
 *   EM_JS_DEPS(mylib_dep, "$allocate,$stackSave");
 *
 * The emscripten linker will then pick this up and make sure those symbols get
 * included in the JS support library.
 *
 * Dependencies declared in this way will be included if-and-only-if the object
 * file (translation unit) in which they exist is included by the linker, so
 * it makes sense co-locate them with the EM_JS or EM_ASM code they correspond
 * to.
 */
#define EM_JS_DEPS(tag, deps)             \
  _EM_BEGIN_CDECL                         \
  EMSCRIPTEN_KEEPALIVE                    \
  __attribute__((section("em_lib_deps"))) \
  __attribute__((aligned(1)))             \
  char __em_lib_deps_##tag[] = deps;      \
  _EM_END_CDECL
