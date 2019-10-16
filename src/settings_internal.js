// Copyright 2010 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tracks whether Emscripten should link in exception throwing (C++ 'throw')
// support library. This does not need to be set directly, but pass
// -fno-exceptions to the build disable exceptions support. (This is basically
// -fno-exceptions, but checked at final link time instead of individual .cpp
// file compile time) If the program *does* contain throwing code (some source
// files were not compiled with `-fno-exceptions`), and this flag is set at link
// time, then you will get errors on undefined symbols, as the exception
// throwing code is not linked in. If so you should either unset the option (if
// you do want exceptions) or fix the compilation of the source files so that
// indeed no exceptions are used).

var DISABLE_EXCEPTION_THROWING = 0;

// An array of all symbols exported from asm.js/wasm module.
var MODULE_EXPORTS = [];

// testing only: Disables the blitOffscreenFramebuffer VAO path.
var OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH = 0;

// testing only: Forces memory growing to fail.
var TEST_MEMORY_GROWTH_FAILS = 0;

// stores the base name of the output file (-o TARGET_BASENAME.js)
var TARGET_BASENAME = '';

// If true, compiler supports setjmp() and longjmp(). If false, these APIs are not available.
// If you are using C++ exceptions, but do not need setjmp()+longjmp() API, then you can set
// this to 0 to save a little bit of code size and performance when catching exceptions.
var SUPPORT_LONGJMP = 1;

// Indicates that the syscalls (which we see statically) indicate that they need full
// filesystem support. Otherwise, when just a small subset are used, we can get away without
// including the full filesystem - in particular, if open() is never used, then we don't
// actually need to support operations on streams.
var SYSCALLS_REQUIRE_FILESYSTEM = 1;

// A list of feature flags to pass to each binaryen invocation (like wasm-opt, etc.). This
// is received from wasm-emscripten-finalize, which reads it from the features section.
var BINARYEN_FEATURES = [];

// Whether EMCC_AUTODEBUG is on, which automatically instruments code for runtime
// logging that can help in debugging.
var AUTODEBUG = 0;

// Whether we should use binaryen's wasm2js to convert our wasm to JS. Set when
// wasm backend is in use with WASM=0 (to enable non-wasm output, we compile to
// wasm normally, then compile that to JS).
var WASM2JS = 0;

// Whether we should link in the runtime for ubsan.
// 0 means do not link ubsan, 1 means link minimal ubsan runtime.
// This is not meant to be used with `-s`. Instead, to use ubsan, use clang flag
// -fsanitize=undefined. To use minimal runtime, also pass `-fsanitize-minimal-runtime`.
var UBSAN_RUNTIME = 0;

// Whether we should link in LSan's runtime library. This is intended to be used
// by -fsanitize=leak instead of used directly.
var USE_LSAN = 0;

// Whether we should link in ASan's runtime library. This is intended to be used
// by -fsanitize=leak instead of used directly.
var USE_ASAN = 0;

// Whether we should load the WASM source map at runtime.
// This is enabled automatically when using -g4 with sanitizers.
var LOAD_SOURCE_MAP = 0;

// Whether embind has been enabled.
var EMBIND = 0;

// Whether the main() function reads the argc/argv parameters.
var MAIN_READS_PARAMS = 1;

// The computed location of the pointer to the sbrk position.
var DYNAMICTOP_PTR = -1;

// The computed initial value of the program break (the sbrk position), which
// is called DYNAMIC_BASE as it is the start of dynamically-allocated memory.
var DYNAMIC_BASE = -1;
