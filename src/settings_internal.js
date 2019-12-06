// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// Settings in this file work exactly like those in settings.js but are not
// set-able from the command line and therefore are not part of the public
// ABI.  This means that these settings are an internal detail of the toolchain
// and can be added/removed/renamed without fear of breaking out users.
//

// An array of all symbols exported from asm.js/wasm module.
var MODULE_EXPORTS = [];

// stores the base name of the output file (-o TARGET_BASENAME.js)
var TARGET_BASENAME = '';

// Indicates that the syscalls (which we see statically) indicate that they need
// full filesystem support. Otherwise, when just a small subset are used, we can
// get away without including the full filesystem - in particular, if open() is
// never used, then we don't actually need to support operations on streams.
var SYSCALLS_REQUIRE_FILESYSTEM = 1;

// A list of feature flags to pass to each binaryen invocation (like wasm-opt,
// etc.). This is received from wasm-emscripten-finalize, which reads it from
// the features section.
var BINARYEN_FEATURES = [];

// Whether EMCC_AUTODEBUG is on, which automatically instruments code for
// runtime logging that can help in debugging.
var AUTODEBUG = 0;

// Whether we should use binaryen's wasm2js to convert our wasm to JS. Set when
// wasm backend is in use with WASM=0 (to enable non-wasm output, we compile to
// wasm normally, then compile that to JS).
var WASM2JS = 0;

// Whether we should link in the runtime for ubsan.
// 0 means do not link ubsan, 1 means link minimal ubsan runtime.
// This is not meant to be used with `-s`. Instead, to use ubsan, use clang flag
// -fsanitize=undefined. To use minimal runtime, also pass
// `-fsanitize-minimal-runtime`.
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

// -Werror was specified on the command line.
var WARNINGS_ARE_ERRORS = 0;

// Temporary setting to force generation and preservation of full DWARF debug info
var FULL_DWARF = 0;
// Maximum seen global alignment; received from the backend.
var MAX_GLOBAL_ALIGN = -1;

// List of functions implemented in compiled code; received from the backend.
var IMPLEMENTED_FUNCTIONS = [];

// Name of the file containing the Fetch *.fetch.js, if relevant
var FETCH_WORKER_FILE = '';
