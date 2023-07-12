//
// @license
// Copyright 2019 The Emscripten Authors
// SPDX-License-Identifier: MIT
//

//
// Settings in this file work exactly like those in settings.js but are not
// set-able from the command line and therefore are not part of the public
// ABI.  This means that these settings are an internal detail of the toolchain
// and can be added/removed/renamed without fear of breaking out users.
//

// List of symbols exported from compiled code
// These are raw symbol names and are not mangled to include the leading
// underscore.
var WASM_EXPORTS = [];

// Similar to above but only includes the global/data symbols.
var WASM_GLOBAL_EXPORTS = [];

// An array of all symbols exported from all the side modules specified on the
// command line.
// These are raw symbol names and are not mangled to include the leading
// underscore.
var SIDE_MODULE_EXPORTS = [];

// All symbols imported by side modules.  These are symbols that the main
// module (or other side modules) will need to provide.
var SIDE_MODULE_IMPORTS = [];

// Like EXPORTED_FUNCTIONS, but will not error if symbol is missing.
// The start/stop symbols are included by default so that then can be extracted
// from the binary and embedded into the generated JS.  The PostEmscripten pass
// in binaryen will then strip these exports so they will not appear in the
// final shipping binary.
// They are included here rather than in REQUIRED_EXPORTS because not all
// programs contains EM_JS or EM_ASM data section, in which case these symbols
// won't exist.
var EXPORT_IF_DEFINED = ['__start_em_asm', '__stop_em_asm',
                         '__start_em_lib_deps', '__stop_em_lib_deps',
                         '__start_em_js', '__stop_em_js'];

// Like EXPORTED_FUNCTIONS, but symbol is required to exist in native code.
// This means wasm-ld will fail if these symbols are missing.
var REQUIRED_EXPORTS = [];

// stores the base name of the output file (-o TARGET_BASENAME.js)
var TARGET_BASENAME = '';

// stores the base name (with extension) of the output JS file
var TARGET_JS_NAME = '';

// Indicates that the syscalls (which we see statically) indicate that they need
// full filesystem support. Otherwise, when just a small subset are used, we can
// get away without including the full filesystem - in particular, if open() is
// never used, then we don't actually need to support operations on streams.
var SYSCALLS_REQUIRE_FILESYSTEM = true;

// A list of feature flags to pass to each binaryen invocation (like wasm-opt,
// etc.). This is received from wasm-emscripten-finalize, which reads it from
// the features section.
var BINARYEN_FEATURES = [];

// Whether EMCC_AUTODEBUG is on, which automatically instruments code for
// runtime logging that can help in debugging.
var AUTODEBUG = false;

// Whether we should use binaryen's wasm2js to convert our wasm to JS. Set when
// wasm backend is in use with WASM=0 (to enable non-wasm output, we compile to
// wasm normally, then compile that to JS).
var WASM2JS = false;

// Whether we should link in the runtime for ubsan.
// 0 means do not link ubsan, 1 means link minimal ubsan runtime.
// This is not meant to be used with `-s`. Instead, to use ubsan, use clang flag
// -fsanitize=undefined. To use minimal runtime, also pass
// `-fsanitize-minimal-runtime`.
var UBSAN_RUNTIME = 0;

// Whether we should link in LSan's runtime library. This is intended to be used
// by -fsanitize=leak instead of used directly.
var USE_LSAN = false;

// Whether we should link in ASan's runtime library. This is intended to be used
// by -fsanitize=leak instead of used directly.
var USE_ASAN = false;

// Whether embind has been enabled.
var EMBIND = false;

// Whether the main() function reads the argc/argv parameters.
var MAIN_READS_PARAMS = true;

// Name of the file containing the Fetch *.fetch.js, if relevant
var FETCH_WORKER_FILE = '';

var WASI_MODULE_NAME = "wasi_snapshot_preview1";

// List of JS libraries explicitly linked against.  This includes JS system
// libraries (specified via -lfoo or -lfoo.js) in addition to user libraries
// passed via `--js-library`.  It does not include implicitly linked libraries
// added by the JS compiler.
var JS_LIBRARIES = [];

// This will contain the emscripten version. This can be useful in combination
// with RETAIN_COMPILER_SETTINGS
var EMSCRIPTEN_VERSION = '';

// Will be set to 0 if -fno-rtti is used on the command line.
var USE_RTTI = true;

// This will contain the optimization level (-Ox).
var OPT_LEVEL = 0;

// This will contain the debug level (-gx).
var DEBUG_LEVEL = 0;

// This will contain the shrink level (1 or 2 for -Os or -Oz, or just 0).
var SHRINK_LEVEL = 0;

// Whether or not to emit the name section in the final wasm binaryen.
var EMIT_NAME_SECTION = false;

// Whether we are emitting a symbol map.
var EMIT_SYMBOL_MAP = false;

// List of function explicitly exported by user on the command line.
var USER_EXPORTED_FUNCTIONS = [];

// name of the file containing wasm binary, if relevant
var WASM_BINARY_FILE = '';

// name of the file containing the pthread *.worker.js, if relevant
var PTHREAD_WORKER_FILE = '';

// name of the file containing the Wasm Worker *.ww.js, if relevant
var WASM_WORKER_FILE = '';

// name of the file containing the Audio Worklet *.aw.js, if relevant
var AUDIO_WORKLET_FILE = '';

// Base URL the source mapfile, if relevant
var SOURCE_MAP_BASE = '';

// When this is false we use an external memory init file
// See --memory-init-file.  When not using wasm2js this flag is ignored, and
// this setting will always be true.
var MEM_INIT_IN_WASM = true;

// If set to 1, src/base64Utils.js will be included in the bundle.
// This is set internally when needed (SINGLE_FILE)
var SUPPORT_BASE64_EMBEDDING = false;

// the possible environments the code may run in.
var ENVIRONMENT_MAY_BE_WEB = true;
var ENVIRONMENT_MAY_BE_WORKER = true;
var ENVIRONMENT_MAY_BE_NODE = true;
var ENVIRONMENT_MAY_BE_SHELL = true;
var ENVIRONMENT_MAY_BE_WEBVIEW = true;

// Whether to minify import and export names in the minify_wasm_js stage.
// Currently always off for MEMORY64.
var MINIFY_WASM_IMPORTS_AND_EXPORTS = false;

// Whether to minify imported module names.
var MINIFY_WASM_IMPORTED_MODULES = false;

// Whether to minify exports from the Wasm module.
var MINIFY_WASM_EXPORT_NAMES = true;

// Internal: represents a browser version that is not supported at all.
var TARGET_NOT_SUPPORTED = 0x7FFFFFFF;

// Used to track whether target environment supports the 'globalThis' attribute.
var SUPPORTS_GLOBALTHIS = false;

// Used to track whether target environment supports the 'Promise.any'.
var SUPPORTS_PROMISE_ANY = false;

// Wasm backend symbols that are considered system symbols and don't
// have the normal C symbol name mangled applied (== prefix with an underscore)
// (Also implicily on this list is any function that starts with string "dynCall_")
var WASM_SYSTEM_EXPORTS = ['stackAlloc', 'stackSave', 'stackRestore', 'getTempRet0', 'setTempRet0'];

// Internal: value of -flto argument (either full or thin)
var LTO = 0;

// Whether we may be accessing the address 2GB or higher. If so, then we need
// to interpret incoming i32 pointers as unsigned.
//
// This setting does not apply (and is never set to true) under MEMORY64, since
// in that case we get 64-bit pointers coming through to JS (converting them to
// i53 in most cases).
var CAN_ADDRESS_2GB = false;

// Whether to emit DWARF in a separate wasm file on the side (this is not called
// "split" because there is already a DWARF concept by that name).
// When DWARF is on the side, the main file has no DWARF info, while the side
// file, ending in .debug.wasm, has the same wasm binary + all the debug
// sections.
// This has no effect if DWARF is not being emitted.
var SEPARATE_DWARF = false;

// New WebAssembly exception handling
var WASM_EXCEPTIONS = false;

// Set to true if the program has a main function.  By default this is
// enabled, but if `--no-entry` is passed, or if `_main` is not part of
// EXPORTED_FUNCTIONS then this gets set to 0.
var EXPECT_MAIN = true;

// Provide and export a .ready() Promise. This is currently used by default with
// MODULARIZE, and returned from the factory function.
var EXPORT_READY_PROMISE = true;

// If true, building against Emscripten's wasm heap memory profiler.
var MEMORYPROFILER = false;

var GENERATE_SOURCE_MAP = false;

var GENERATE_DWARF = false;

// Memory layout.  These are only used/set in RELOCATABLE builds.  Otherwise
// memory layout is fixed in the wasm binary at link time.
var STACK_HIGH = 0;
var STACK_LOW = 0;
var HEAP_BASE = 0;

// Used internally. set when there is a main() function.
// Also set when in a linkable module, as the main() function might
// arrive from a dynamically-linked library, and not necessarily
// the current compilation unit.
// Also set for STANDALONE_WASM since the _start function is needed to call
// static ctors, even if there is no user main.
var HAS_MAIN = false;

// Set to true if we are linking as C++ and including C++ stdlibs
var LINK_AS_CXX = false;

// Set when some minimum browser version triggers doesn't support the
// minimum set of ES6 features.  This triggers transpilation to ES5
// using closure compiler.
var TRANSPILE_TO_ES5 = false;

// A copy of the default the default INCOMING_MODULE_JS_API. (Soon to
// include additional items).
var ALL_INCOMING_MODULE_JS_API = [];

// List of all imports that are weak, and therefore allowed to be undefined at
// runtime.  This is used by the JS compiler to avoid build-time warnings/errors
// when weak symbols are undefined.  Only applies in the case of dyanmic linking
// (MAIN_MODULE).
var WEAK_IMPORTS = [];

var STACK_FIRST = false;

var HAVE_EM_ASM = true;

var PRE_JS_FILES = [];

var POST_JS_FILES = [];

// Set when -pthread / -sPTHREADS is passed
var PTHREADS = false;

var BULK_MEMORY = false;
