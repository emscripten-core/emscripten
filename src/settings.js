//
// @license
// Copyright 2010 The Emscripten Authors
// SPDX-License-Identifier: MIT
//

//
// Various compiler settings. These are simply variables present when the
// JS compiler runs. To set them, do something like:
//
//   emcc -s OPTION1=VALUE1 -s OPTION2=VALUE2 [..other stuff..]
//
// For convenience and readability `-s OPTION` expands to `-s OPTION=1`
// and `-s NO_OPTION` expands to `-s OPTION=0` (assuming OPTION is a valid
// option).
//
// See https://github.com/emscripten-core/emscripten/wiki/Code-Generation-Modes/
//
// Note that the values here are the defaults in -O0, that is, unoptimized
// mode. See apply_opt_level in tools/shared.py for how -O1,2,3 affect these
// flags.
//
// These flags should only have an effect when compiling to JS, so there
// should not be a need to have them when just compiling source to
// bitcode. However, there will also be no harm either, so it is ok to.
//
// Settings in this file can be directly set from the command line.  Internal
// settings that are not part of the user ABI live in the settings_internal.js.
//
// In general it is best to pass the same arguments at both compile and link
// time, as whether wasm object files are used or not affects when codegen
// happens (without wasm object files, codegen is done entirely during
// link; otherwise, it is during compile). Flags affecting codegen must
// be passed when codegen happens, so to let a build easily switch when codegen
// happens (LTO vs normal), pass the flags at both times. The flags are also
// annotated in this file:
//
// [link] - Should be passed at link time. This is the case for all JS flags,
//          as we emit JS at link (and that is most of the flags here, and
//          hence the default).
// [compile+link] - A flag that has an effect at both compile and link time,
//                  basically any time emcc is invoked. The same flag should be
//                  passed at both times in most cases.
//
// If not otherwise specified, a flag is [link]. Note that no flag is only
// relevant during compile time, as during link we may do codegen for system
// libraries and other support code, so all flags are either link or
// compile+link.
//

// Tuning

// Whether we should add runtime assertions, for example to
// check that each allocation to the stack does not
// exceed its size, whether all allocations (stack and static) are
// of positive size, etc., whether we should throw if we encounter a bad __label__, i.e.,
// if code flow runs into a fault
// ASSERTIONS == 2 gives even more runtime checks, that may be very slow. That
// includes internal dlmalloc assertions.
// [link]
var ASSERTIONS = 1;

// Whether extra logging should be enabled.
// This logging isn't quite assertion-quality in that it isn't necessarily a
// symptom that something is wrong.
// [link]
var RUNTIME_LOGGING = 0;

// Chooses what kind of stack smash checks to emit to generated code:
// Building with ASSERTIONS=1 causes STACK_OVERFLOW_CHECK default to 1.
// Since ASSERTIONS=1 is the default at -O0, which itself is the default
// optimization level this means that this setting also effectively
// defaults 1, absent any other settings.
// 0: Stack overflows are not checked.
// 1: Adds a security cookie at the top of the stack, which is checked at end of
//    each tick and at exit (practically zero performance overhead)
// 2: Same as above, but also runs a binaryen pass which adds a check to all
//    stack pointer assignments. Has a small performance cost.
// [link]
var STACK_OVERFLOW_CHECK = 0;

// When set to 1, will generate more verbose output during compilation.
// [general]
var VERBOSE = 0;

// Whether we will run the main() function. Disable if you embed the generated
// code in your own, and will call main() yourself at the right time (which you
// can do with Module.callMain(), with an optional parameter of commandline args).
// [link]
var INVOKE_RUN = 1;

// If 0, the runtime is not quit when main() completes (allowing code to
// run afterwards, for example from the browser main event loop). atexit()s
// are also not executed, and we can avoid including code for runtime shutdown,
// like flushing the stdio streams.
// Set this to 1 if you do want atexit()s or stdio streams to be flushed
// on exit.
// This setting is controlled automatically in STANDALONE_WASM mode:
//  - For a command (has a main function) this is always 1
//  - For a reactor (no a main function) this is always 0
// [link]
var EXIT_RUNTIME = 0;

// How to represent the initial memory content.
// 0: embed a base64 string literal representing the initial memory data
// 1: create a *.mem file containing the binary data of the initial memory;

//    use the --memory-init-file command line switch to select this method
// [link]
var MEM_INIT_METHOD = 0;

// The total stack size. There is no way to enlarge the stack, so this
// value must be large enough for the program's requirements. If
// assertions are on, we will assert on not exceeding this, otherwise,
// it will fail silently.
// [link]
var TOTAL_STACK = 5*1024*1024;

// What malloc()/free() to use, out of
//  * dlmalloc - a powerful general-purpose malloc
//  * emmalloc - a simple and compact malloc designed for emscripten
//  * emmalloc-debug - use emmalloc and add extra assertion checks
//  * emmalloc-memvalidate - use emmalloc with assertions+heap consistency
//                           checking.
//  * emmalloc-verbose - use emmalloc with assertions + verbose logging.
//  * emmalloc-memvalidate-verbose - use emmalloc with assertions + heap
//                                   consistency checking + verbose logging.
//  * none     - no malloc() implementation is provided, but you must implement
//               malloc() and free() yourself.
// dlmalloc is necessary for split memory and other special modes, and will be
// used automatically in those cases.
// In general, if you don't need one of those special modes, and if you don't
// allocate very many small objects, you should use emmalloc since it's
// smaller. Otherwise, if you do allocate many small objects, dlmalloc
// is usually worth the extra size. dlmalloc is also a good choice if you want
// the extra security checks it does (such as noticing metadata corruption in
// its internal data structures, which emmalloc does not do).
// [link]
var MALLOC = "dlmalloc";

// If 1, then when malloc would fail we abort(). This is nonstandard behavior,
// but makes sense for the web since we have a fixed amount of memory that
// must all be allocated up front, and so (a) failing mallocs are much more
// likely than on other platforms, and (b) people need a way to find out
// how big that initial allocation (INITIAL_MEMORY) must be.
// If you set this to 0, then you get the standard malloc behavior of
// returning NULL (0) when it fails.
//
// Setting ALLOW_MEMORY_GROWTH turns this off, as in that mode we default to
// the behavior of trying to grow and returning 0 from malloc on failure, like
// a standard system would. However, you can still set this flag to override
// that.
//    * This is a mostly-backwards-compatible change. Previously this option
//      was ignored when growth was on. The current behavior is that growth
//      turns it off by default, so for users that never specified the flag
//      nothing changes. But if you do specify it, it will have an effect now,
//      which it did not previously. If you don't want that, just stop passing
//      it in at link time.
//
// [link]
var ABORTING_MALLOC = 1;

// The initial amount of memory to use. Using more memory than this will
// cause us to expand the heap, which can be costly with typed arrays:
// we need to copy the old heap into a new one in that case.
// If ALLOW_MEMORY_GROWTH is set, this initial amount of memory can increase
// later; if not, then it is the final and total amount of memory.
//
// (This option was formerly called TOTAL_MEMORY.)
// [link]
var INITIAL_MEMORY = 16777216;

// Set the maximum size of memory in the wasm module (in bytes). This is only
// relevant when ALLOW_MEMORY_GROWTH is set, as without growth, the size of
// INITIAL_MEMORY is the final size of memory anyhow.
//
// If this value is -1, it means there is no specified limit.
//
// Note that the default value here is 2GB, which means that by default if you
// enable memory growth then we can grow up to 2GB but no higher. 2GB is a
// natural limit for several reasons:
//
//   * If the maximum heap size is over 2GB, then pointers must be unsigned in
//     JavaScript, which increases code size. We don't want memory growth builds
//     to be larger unless someone explicitly opts in to >2GB+ heaps.
//   * Historically no VM has supported more >2GB+, and only recently (Mar 2020)
//     has support started to appear. As support is limited, it's safer for
//     people to opt into >2GB+ heaps rather than get a build that may not
//     work on all VMs.
//
// (This option was formerly called WASM_MEM_MAX and BINARYEN_MEM_MAX.)
// [link]
var MAXIMUM_MEMORY = 2147483648;

// If false, we abort with an error if we try to allocate more memory than
// we can (INITIAL_MEMORY). If true, we will grow the memory arrays at
// runtime, seamlessly and dynamically. 
// See https://code.google.com/p/v8/issues/detail?id=3907 regarding
// memory growth performance in chrome.
// Note that growing memory means we replace the JS typed array views, as
// once created they cannot be resized. (In wasm we can grow the Memory, but
// still need to create new views for JS.)
// Setting this option on will disable ABORTING_MALLOC, in other words,
// ALLOW_MEMORY_GROWTH enables fully standard behavior, of both malloc
// returning 0 when it fails, and also of being able to allocate more
// memory from the system as necessary.
// [link]
var ALLOW_MEMORY_GROWTH = 0;

// If ALLOW_MEMORY_GROWTH is true, this variable specifies the geometric
// overgrowth rate of the heap at resize. Specify MEMORY_GROWTH_GEOMETRIC_STEP=0
// to disable overgrowing the heap at all, or e.g.
// MEMORY_GROWTH_GEOMETRIC_STEP=1.0 to double the heap (+100%) at every grow step.
// The larger this value is, the more memory the WebAssembly heap overreserves
// to reduce performance hiccups coming from memory resize, and the smaller
// this value is, the more memory is conserved, at the performance of more
// stuttering when the heap grows. (profiled to be on the order of ~20 msecs)
// [link]
var MEMORY_GROWTH_GEOMETRIC_STEP = 0.20;

// Specifies a cap for the maximum geometric overgrowth size, in bytes. Use
// this value to constrain the geometric grow to not exceed a specific rate.
// Pass MEMORY_GROWTH_GEOMETRIC_CAP=0 to disable the cap and allow unbounded
// size increases.
// [link]
var MEMORY_GROWTH_GEOMETRIC_CAP = 96*1024*1024;

// If ALLOW_MEMORY_GROWTH is true and MEMORY_GROWTH_LINEAR_STEP == -1, then
// geometric memory overgrowth is utilized (above variable). Set
// MEMORY_GROWTH_LINEAR_STEP to a multiple of WASM page size (64KB), eg. 16MB to
// replace geometric overgrowth rate with a constant growth step size. When
// MEMORY_GROWTH_LINEAR_STEP is used, the variables MEMORY_GROWTH_GEOMETRIC_STEP
// and MEMORY_GROWTH_GEOMETRIC_CAP are ignored.
// [link]
var MEMORY_GROWTH_LINEAR_STEP = -1;

// The "architecture" to compile for. 0 means the default wasm32, 1 is
// the full end-to-end wasm64 mode, and 2 is wasm64 for clang/lld but lowered to
// wasm32 in Binaryen (such that it can run on wasm32 engines, while internally
// using i64 pointers).
// [compile+link]
var MEMORY64 = 0;

// Sets the initial size of the table when MAIN_MODULE or SIDE_MODULE is use
// (and not otherwise). Normally Emscripten can determine the size of the table
// at link time, but in SPLIT_MODULE mode, wasm-split often needs to grow the
// table, so the table size baked into the JS for the instrumented build will be
// too small after the module is split. This is a hack to allow users to specify
// a large enough table size that can be consistent across both builds. This
// setting may be removed at any time and should not be used except in
// conjunction with SPLIT_MODULE and dynamic linking.
// [link]
var INITIAL_TABLE = -1;

// If true, allows more functions to be added to the table at runtime. This is
// necessary for dynamic linking, and set automatically in that mode.
// [link]
var ALLOW_TABLE_GROWTH = 0;

// where global data begins; the start of static memory. -1 means use the
// default, any other value will be used as an override
// [link]
var GLOBAL_BASE = -1;

// Whether closure compiling is being run on this output
// [link]
var USE_CLOSURE_COMPILER = 0;

// Specifies how warnings emitted by Closure are treated. Possible
// options: 'quiet', 'warn', 'error'. If set to 'warn', Closure warnings are printed
// out to console. If set to 'error', Closure warnings are treated like errors,
// similar to -Werror compiler flag.
// [link]
var CLOSURE_WARNINGS = 'quiet';

// Ignore closure warnings and errors (like on duplicate definitions)
// [link]
var IGNORE_CLOSURE_COMPILER_ERRORS = 0;

// If set to 1, each wasm module export is individually declared with a
// JavaScript "var" definition. This is the simple and recommended approach.
// However, this does increase code size (especially if you have many such
// exports), which can be avoided in an unsafe way by setting this to 0. In that
// case, no "var" is created for each export, and instead a loop (of small
// constant code size, no matter how many exports you have) writes all the
// exports received into the global scope. Doing so is dangerous since such
// modifications of the global scope can confuse external JS minifier tools, and
// also things can break if the scope the code is in is not the global scope
// (e.g. if you manually enclose them in a function scope).
// [link]
var DECLARE_ASM_MODULE_EXPORTS = 1;

// A limit on inlining. If 0, we will inline normally in LLVM and closure. If
// greater than 0, we will *not* inline in LLVM, and we will prevent inlining of
// functions of this size or larger in closure. 50 is a reasonable setting if
// you do not want inlining
// [compile+link]
var INLINING_LIMIT = 0;

// If set to 1, perform acorn pass that converts each HEAP access into a
// function call that uses DataView to enforce LE byte order for HEAP buffer;
// This makes generated JavaScript run on BE as well as LE machines. (If 0, only
// LE systems are supported). Does not affect generated wasm.
var SUPPORT_BIG_ENDIAN = 0;

// Check each write to the heap, for example, this will give a clear
// error on what would be segfaults in a native build (like dereferencing
// 0). See runtime_safe_heap.js for the actual checks performed.
// [link]
var SAFE_HEAP = 0;

// Log out all SAFE_HEAP operations
// [link]
var SAFE_HEAP_LOG = 0;

// Allows function pointers to be cast, wraps each call of an incorrect type
// with a runtime correction.  This adds overhead and should not be used
// normally.  It also forces ALIASING_FUNCTION_POINTERS to 0.  Aside from making
// calls not fail, this tries to convert values as best it can.
// We use 64 bits (i64) to represent values, as if we wrote the sent value to
// memory and loaded the received type from the same memory (using
// truncs/extends/ reinterprets). This means that when types do not match the
// emulated values may not match (this is true of native too, for that matter -
// this is all undefined behavior). This approaches appears good enough to
// support Python, which is the main use case motivating this feature.
// [link]
var EMULATE_FUNCTION_POINTER_CASTS = 0;

// Print out exceptions in emscriptened code.
// [link]
var EXCEPTION_DEBUG = 0;

// If 1, build in libcxxabi's full c++ demangling code, to allow stackTrace()
// to emit fully proper demangled c++ names
// [link]
var DEMANGLE_SUPPORT = 0;

// Print out when we enter a library call (library*.js). You can also unset
// Runtime.debug at runtime for logging to cease, and can set it when you want
// it back. A simple way to set it in C++ is
//   emscripten_run_script("Runtime.debug = ...;");
// [link]
var LIBRARY_DEBUG = 0;

// Print out all musl syscalls, including translating their numeric index
// to the string name, which can be convenient for debugging. (Other system
// calls are not numbered and already have clear names; use LIBRARY_DEBUG
// to get logging for all of them.)
// [link]
var SYSCALL_DEBUG = 0;

// Log out socket/network data transfer.
// [link]
var SOCKET_DEBUG = 0;

// Log dynamic linker information
// [link]
var DYLINK_DEBUG = 0;

// Select socket backend, either webrtc or websockets. XXX webrtc is not
// currently tested, may be broken

// As well as being configurable at compile time via the "-s" option the
// WEBSOCKET_URL and WEBSOCKET_SUBPROTOCOL
// settings may configured at run time via the Module object e.g.
// Module['websocket'] = {subprotocol: 'base64, binary, text'};
// Module['websocket'] = {url: 'wss://', subprotocol: 'base64'};
// You can set 'subprotocol' to null, if you don't want to specify it
// Run time configuration may be useful as it lets an application select
// multiple different services.
// [link]
var SOCKET_WEBRTC = 0;

// A string containing either a WebSocket URL prefix (ws:// or wss://) or a complete
// RFC 6455 URL - "ws[s]:" "//" host [ ":" port ] path [ "?" query ].
// In the (default) case of only a prefix being specified the URL will be constructed from
// prefix + addr + ':' + port
// where addr and port are derived from the socket connect/bind/accept calls.
// [link]
var WEBSOCKET_URL = 'ws://';

// If 1, the POSIX sockets API uses a native bridge process server to proxy sockets calls
// from browser to native world.
// [link]
var PROXY_POSIX_SOCKETS = 0;

// A string containing a comma separated list of WebSocket subprotocols
// as would be present in the Sec-WebSocket-Protocol header.
// You can set 'null', if you don't want to specify it.
// [link]
var WEBSOCKET_SUBPROTOCOL = 'binary';

// Print out debugging information from our OpenAL implementation.
// [link]
var OPENAL_DEBUG = 0;

// If 1, prints out debugging related to calls from emscripten_web_socket_* functions
// in emscripten/websocket.h.
// If 2, additionally traces bytes communicated via the sockets.
// [link]
var WEBSOCKET_DEBUG = 0;

// Adds extra checks for error situations in the GL library. Can impact
// performance.
// [link]
var GL_ASSERTIONS = 0;

// If enabled, prints out all API calls to WebGL contexts. (*very* verbose)
// [link]
var TRACE_WEBGL_CALLS = 0;

// Enables more verbose debug printing of WebGL related operations. As with
// LIBRARY_DEBUG, this is toggleable at runtime with option GL.debug.
// [link]
var GL_DEBUG = 0;

// When enabled, sets preserveDrawingBuffer in the context, to allow tests to
// work (but adds overhead)
// [link]
var GL_TESTING = 0;

// How large GL emulation temp buffers are
// [link]
var GL_MAX_TEMP_BUFFER_SIZE = 2097152;

// Enables some potentially-unsafe optimizations in GL emulation code
// [link]
var GL_UNSAFE_OPTS = 1;

// Forces support for all GLES2 features, not just the WebGL-friendly subset.
// [link]
var FULL_ES2 = 0;

// If true, glGetString() for GL_VERSION and GL_SHADING_LANGUAGE_VERSION will
// return strings OpenGL ES format "Open GL ES ... (WebGL ...)" rather than the
// WebGL format. If false, the direct WebGL format strings are returned. Set
// this to true to make GL contexts appear like an OpenGL ES context in these
// version strings (at the expense of a little bit of added code size), and to
// false to make GL contexts appear like WebGL contexts and to save some bytes
// from the output.
// [link]
var GL_EMULATE_GLES_VERSION_STRING_FORMAT = 1;

// If true, all GL extensions are advertised in both unprefixed WebGL extension
// format, but also in desktop/mobile GLES/GL extension format with "GL_" prefix.
// [link]
var GL_EXTENSIONS_IN_PREFIXED_FORMAT = 1;

// If true, adds support for automatically enabling all GL extensions for
// GLES/GL emulation purposes. This takes up code size. If you set this to 0,
// you will need to manually enable the extensions you need.
// [link]
var GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS = 1;

// If true, the function emscripten_webgl_enable_extension() can be called to
// enable any WebGL extension. If false, to save code size,
// emscripten_webgl_enable_extension() cannot be called to enable any of extensions
// 'ANGLE_instanced_arrays', 'OES_vertex_array_object', 'WEBGL_draw_buffers',
// 'WEBGL_multi_draw', 'WEBGL_draw_instanced_base_vertex_base_instance',
// or 'WEBGL_multi_draw_instanced_base_vertex_base_instance',
// but the dedicated functions emscripten_webgl_enable_*()
// found in html5.h are used to enable each of those extensions.
// This way code size is increased only for the extensions that are actually used.
// N.B. if setting this to 0, GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS must be set
// to zero as well.
// [link]
var GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS = 1;

// If set to 0, Emscripten GLES2->WebGL translation layer does not track the kind
// of GL errors that exist in GLES2 but do not exist in WebGL. Settings this to 0
// saves code size. (Good to keep at 1 for development)
// [link]
var GL_TRACK_ERRORS = 1;

// If true, GL contexts support the explicitSwapControl context creation flag.
// Set to 0 to save a little bit of space on projects that do not need it.
// [link]
var GL_SUPPORT_EXPLICIT_SWAP_CONTROL = 0;

// If true, calls to glUniform*fv and glUniformMatrix*fv utilize a pool of
// preallocated temporary buffers for common small sizes to avoid generating
// temporary garbage for WebGL 1. Disable this to optimize generated size of the
// GL library a little bit, at the expense of generating garbage in WebGL 1. If
// you are only using WebGL 2 and do not support WebGL 1, this is not needed and
// you can turn it off.
// [link]
var GL_POOL_TEMP_BUFFERS = 1;

// Some old Android WeChat (Chromium 37?) browser has a WebGL bug that it ignores
// the offset of a typed array view pointing to an ArrayBuffer. Set this to
// 1 to enable a polyfill that works around the issue when it appears. This
// bug is only relevant to WebGL 1, the affected browsers do not support WebGL 2.
// [link]
var WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG = 0;

// Deprecated. Pass -s MAX_WEBGL_VERSION=2 to target WebGL 2.0.
// [link]
var USE_WEBGL2 = 0;

// Specifies the lowest WebGL version to target. Pass -s MIN_WEBGL_VERSION=1
// to enable targeting WebGL 1, and -s MIN_WEBGL_VERSION=2 to drop support
// for WebGL 1.0
// [link]
var MIN_WEBGL_VERSION = 1;

// Specifies the highest WebGL version to target. Pass -s MAX_WEBGL_VERSION=2
// to enable targeting WebGL 2. If WebGL 2 is enabled, some APIs (EGL, GLUT, SDL)
// will default to creating a WebGL 2 context if no version is specified.
// Note that there is no automatic fallback to WebGL1 if WebGL2 is not supported
// by the user's device, even if you build with both WebGL1 and WebGL2
// support, as that may not always be what the application wants. If you want
// such a fallback, you can try to create a context with WebGL2, and if that
// fails try to create one with WebGL1.
// [link]
var MAX_WEBGL_VERSION = 1;

// If true, emulates some WebGL 1 features on WebGL 2 contexts, meaning that
// applications that use WebGL 1/GLES 2 can initialize a WebGL 2/GLES3 context,
// but still keep using WebGL1/GLES 2 functionality that no longer is supported
// in WebGL2/GLES3. Currently this emulates GL_EXT_shader_texture_lod extension
// in GLSLES 1.00 shaders, support for unsized internal texture formats, and the
// GL_HALF_FLOAT_OES != GL_HALF_FLOAT mixup.
// [link]
var WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION = 0;

// Forces support for all GLES3 features, not just the WebGL2-friendly subset.
// This automatically turns on FULL_ES2 and WebGL2 support.
// [link]
var FULL_ES3 = 0;

// Includes code to emulate various desktop GL features. Incomplete but useful
// in some cases, see
// http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
// [link]
var LEGACY_GL_EMULATION = 0;

// If you specified LEGACY_GL_EMULATION = 1 and only use fixed function pipeline
// in your code, you can also set this to 1 to signal the GL emulation layer
// that it can perform extra optimizations by knowing that the user code does
// not use shaders at all. If LEGACY_GL_EMULATION = 0, this setting has no
// effect.
// [link]
var GL_FFP_ONLY = 0;

// If you want to create the WebGL context up front in JS code, set this to 1
// and set Module['preinitializedWebGLContext'] to a precreated WebGL context.
// WebGL initialization afterwards will use this GL context to render.
// [link]
var GL_PREINITIALIZED_CONTEXT = 0;

// Enables support for WebGPU (via "webgpu/webgpu.h").
// [link]
var USE_WEBGPU = 0;

// Enables building of stb-image, a tiny public-domain library for decoding
// images, allowing decoding of images without using the browser's built-in
// decoders. The benefit is that this can be done synchronously, however, it
// will not be as fast as the browser itself.  When enabled, stb-image will be
// used automatically from IMG_Load and IMG_Load_RW. You can also call the
// stbi_* functions directly yourself.
// [link]
var STB_IMAGE = 0;

// From Safari 8 (where WebGL was introduced to Safari) onwards, OES_texture_half_float and OES_texture_half_float_linear extensions
// are broken and do not function correctly, when used as source textures.
// See https://bugs.webkit.org/show_bug.cgi?id=183321, https://bugs.webkit.org/show_bug.cgi?id=169999,
// https://stackoverflow.com/questions/54248633/cannot-create-half-float-oes-texture-from-uint16array-on-ipad
// [link]
var GL_DISABLE_HALF_FLOAT_EXTENSION_IF_BROKEN = 0;

// Workaround Safari WebGL issue: After successfully acquiring WebGL context on a canvas,
// calling .getContext() will always return that context independent of which 'webgl' or 'webgl2'
// context version was passed. See https://bugs.webkit.org/show_bug.cgi?id=222758 and
// https://github.com/emscripten-core/emscripten/issues/13295.
// Set this to 0 to force-disable the workaround if you know the issue will not affect you.
var GL_WORKAROUND_SAFARI_GETCONTEXT_BUG = 1;

// Use JavaScript math functions like Math.tan. This saves code size as we can avoid shipping
// compiled musl code. However, it can be significantly slower as it calls out to JS. It
// also may give different results as JS math is specced somewhat differently than libc, and
// can also vary between browsers.
// [link]
var JS_MATH = 0;

// If set, enables polyfilling for Math.clz32, Math.trunc, Math.imul, Math.fround.
// [link]
var POLYFILL_OLD_MATH_FUNCTIONS = 0;

// Set this to enable compatibility emulations for old JavaScript engines. This gives you
// the highest possible probability of the code working everywhere, even in rare old
// browsers and shell environments. Specifically:
//  * Add polyfilling for Math.clz32, Math.trunc, Math.imul, Math.fround. (-s POLYFILL_OLD_MATH_FUNCTIONS=1)
//  * Work around old Chromium WebGL 1 bug (-s WORKAROUND_OLD_WEBGL_UNIFORM_UPLOAD_IGNORED_OFFSET_BUG=1)
//  * Disable WebAssembly. (Must be paired with -s WASM=0)
//  * Adjusts MIN_X_VERSION settings to 0 to include support for all browser versions.
// You can also configure the above options individually.
// [link]
var LEGACY_VM_SUPPORT = 0;

// By default, emscripten output will run on the web, in a web worker,
// in node.js, or in a JS shell like d8, js, or jsc. You can set this option to
// specify that the output should only run in one particular environment, which
// must be one of
//    'web'     - the normal web environment.
//    'webview' - just like web, but in a webview like Cordova;
//                considered to be same as "web" in almost every place
//    'worker'  - a web worker environment.
//    'node'    - Node.js.
//    'shell'   - a JS shell like d8, js, or jsc.
// Or it can be a comma-separated list of them, e.g., "web,worker". If this is
// the empty string, then all runtime environments are supported.
//
// Note that the set of environments recognized here is not identical to the
// ones we identify at runtime using ENVIRONMENT_IS_*. Specifically:
//  * We detect whether we are a pthread at runtime, but that's set for workers
//    and not for the main file so it wouldn't make sense to specify here.
//  * The webview target is basically a subset of web. It must be specified
//    alongside web (e.g. "web,webview") and we only use it for code generation
//    at compile time, there is no runtime behavior change.
// [link]
var ENVIRONMENT = '';

// Enable this to support lz4-compressed file packages. They are stored compressed in memory, and
// decompressed on the fly, avoiding storing the entire decompressed data in memory at once.
// If you run the file packager separately, you still need to build the main program with this flag,
// and also pass --lz4 to the file packager.
// (You can also manually compress one on the client, using LZ4.loadPackage(), but that is less
// recommended.)
// Limitations:
//   * LZ4-compressed files are only decompressed when needed, so they are not available
//     for special preloading operations like pre-decoding of images using browser codecs,
//     preloadPlugin stuff, etc.
//   * LZ4 files are read-only.
// [link]
var LZ4 = 0;

// Emscripten exception handling options.
// These options only pertain to Emscripten exception handling and do not
// control the experimental native wasm exception handling option.

// Disables generating code to actually catch exceptions. This disabling is on
// by default as the overhead of exceptions is quite high in size and speed
// currently (in the future, wasm should improve that). When exceptions are
// disabled, if an exception actually happens then it will not be caught
// and the program will halt (so this will not introduce silent failures).
//
// XXX note that this removes *catching* of exceptions, which is the main
//     issue for speed, but you should build source files with
//     -fno-exceptions to really get rid of all exceptions code overhead,
//     as it may contain thrown exceptions that are never caught (e.g.
//     just using std::vector can have that). -fno-rtti may help as well.
//
// This option is mutually exclusive with EXCEPTION_CATCHING_ALLOWED.
//
// [compile+link] - affects user code at compile and system libraries at link
var DISABLE_EXCEPTION_CATCHING = 1;

// Enables catching exception but only in the listed functions.  This
// option acts like a more precise version of `DISABLE_EXCEPTION_CATCHING=0`.
//
// This option is mutually exclusive with DISABLE_EXCEPTION_CATCHING.
//
// [compile+link] - affects user code at compile and system libraries at link
var EXCEPTION_CATCHING_ALLOWED = [];

// By default we handle exit() in node, by catching the Exit exception. However,
// this means we catch all process exceptions. If you disable this, then we no
// longer do that, and exceptions work normally, which can be useful for libraries
// or programs that don't need exit() to work.

// Emscripten uses an ExitStatus exception to halt when exit() is called.
// With this option, we prevent that from showing up as an unhandled
// exception.
// [link]
var NODEJS_CATCH_EXIT = 1;

// Catch unhandled rejections in node. Without this, node may print the error,
// and that this behavior will change in future node, wait a few seconds, and
// then exit with 0 (which hides the error if you don't read the log). With
// this, we catch any unhandled rejection and throw an actual error, which will
// make the process exit immediately with a non-0 return code.
// This should be fixed in Node 15+.
// [link]
var NODEJS_CATCH_REJECTION = 1;

// Whether to transform the code using asyncify. This makes it possible to
// call JS functions from synchronous-looking code in C/C++.
// See https://emscripten.org/docs/porting/asyncify.html
// [link]
var ASYNCIFY = 0;

// Imports which can do an sync operation, in addition to the default ones that
// emscripten defines like emscripten_sleep. If you add more you will need to
// mention them to here, or else they will not work (in ASSERTIONS builds an
// error will be shown).
// Note that this list used to contain the default ones, which meant that you
// had to list them when adding your own; the default ones are now added
// automatically.
// [link]
var ASYNCIFY_IMPORTS = [];

// Whether indirect calls can be on the stack during an unwind/rewind.
// If you know they cannot, then setting this can be extremely helpful, as otherwise asyncify
// must assume an indirect call can reach almost everywhere.
// [link]
var ASYNCIFY_IGNORE_INDIRECT = 0;

// The size of the asyncify stack - the region used to store unwind/rewind
// info. This must be large enough to store the call stack and locals. If it is too
// small, you will see a wasm trap due to executing an "unreachable" instruction.
// In that case, you should increase this size.
// [link]
var ASYNCIFY_STACK_SIZE = 4096;

// If the Asyncify remove-list is provided, then the functions in it will not
// be instrumented even if it looks like they need to. This can be useful
// if you know things the whole-program analysis doesn't, like if you
// know certain indirect calls are safe and won't unwind. But if you
// get the list wrong things will break (and in a production build user
// input might reach code paths you missed during testing, so it's hard
// to know you got this right), so this is not recommended unless you
// really know what are doing, and need to optimize every bit of speed
// and size.
// The names in this list are names from the WebAssembly Names section. The
// wasm backend will emit those names in *human-readable* form instead of
// typical C++ mangling. For example, you should write Struct::func()
// instead of _ZN6Struct4FuncEv. C is also different from C++, as C
// names don't end with parameters; as a result foo(int) in C++ would appear
// as just foo in C (C++ has parameters because it needs to differentiate
// overloaded functions). You will see warnings in the console if a name in the
// list is missing (these are not errors because inlining etc. may cause
// changes which would mean a single list couldn't work for both -O0 and -O1
// builds, etc.). You can inspect the wasm binary to look for the actual names,
// either directly or using wasm-objdump or wasm-dis, etc.
// Simple '*' wildcard matching is supported.
// [link]
var ASYNCIFY_REMOVE = [];

// Functions in the Asyncify add-list are added to the list of instrumented
// functions, that is, they will be instrumented even if otherwise asyncify
// thinks they don't need to be. As by default everything will be instrumented
// in the safest way possible, this is only useful if you use IGNORE_INDIRECT
// and use this list to fix up some indirect calls that *do* need to be
// instrumented.
// See notes on ASYNCIFY_REMOVE about the names.
// [link]
var ASYNCIFY_ADD = [];

// If the Asyncify only-list is provided, then *only* the functions in the list
// will be instrumented. Like the remove-list, getting this wrong will break
// your application.
// See notes on ASYNCIFY_REMOVE about the names.
// [link]
var ASYNCIFY_ONLY = [];

// If enabled will output which functions have been instrumented and why.
// [link]
var ASYNCIFY_ADVISE = 0;

// Allows lazy code loading: where emscripten_lazy_load_code() is written, we
// will pause execution, load the rest of the code, and then resume.
// [link]
var ASYNCIFY_LAZY_LOAD_CODE = 0;

// Runtime debug logging from asyncify internals.
// [link]
var ASYNCIFY_DEBUG = 0;

// Runtime elements that are exported on Module by default. We used to export
// quite a lot here, but have removed them all, so this option is redundant
// given that EXTRA_EXPORTED_RUNTIME_METHODS exists, and so this option exists
// only for backwards compatibility. You should use
// EXTRA_EXPORTED_RUNTIME_METHODS for things you want to export from the
// runtime.  Note that methods on this list are only exported if they are
// included (either automatically from linking, or due to being in
// DEFAULT_LIBRARY_FUNCS_TO_INCLUDE).
// Note that the name may be slightly misleading, as this is for any JS library
// element, and not just methods. For example, we can export the FS object by
// having "FS" in this list.
// [link]
var EXPORTED_RUNTIME_METHODS = [];

// Additional methods to those in EXPORTED_RUNTIME_METHODS. Adjusting that list
// lets you remove methods that would be exported by default; setting values in
// this list lets you add to the default list without modifying it.
// [link]
var EXTRA_EXPORTED_RUNTIME_METHODS = [];

// A list of incoming values on the Module object in JS that we care about. If
// a value is not in this list, then we don't emit code to check if you provide
// it on the Module object. For example, if
// you have this:
//
//  var Module = {
//    print: function(x) { console.log('print: ' + x) },
//    preRun: [function() { console.log('pre run') }]
//  };
//
// Then MODULE_JS_API must contain 'print' and 'preRun'; if it does not then
// we may not emit code to read and use that value. In other words, this
// option lets you set, statically at compile time, the list of which Module
// JS values you will be providing at runtime, so the compiler can better
// optimize.
//
// Setting this list to [], or at least a short and concise set of names you
// actually use, can be very useful for reducing code size. By default the
// list contains all the possible APIs.
//
// FIXME: should this just be  0  if we want everything?
// [link]
var INCOMING_MODULE_JS_API = [
  'ENVIRONMENT', 'GL_MAX_TEXTURE_IMAGE_UNITS', 'SDL_canPlayWithWebAudio',
  'SDL_numSimultaneouslyQueuedBuffers', 'INITIAL_MEMORY', 'wasmMemory', 'arguments',
  'buffer', 'canvas', 'doNotCaptureKeyboard', 'dynamicLibraries',
  'elementPointerLock', 'extraStackTrace', 'forcedAspectRatio',
  'instantiateWasm', 'keyboardListeningElement', 'freePreloadedMediaOnUse',
  'locateFile', 'logReadFiles', 'mainScriptUrlOrBlob', 'mem',
  'monitorRunDependencies', 'noExitRuntime', 'noInitialRun', 'onAbort',
  'onCustomMessage', 'onExit', 'onFree', 'onFullScreen', 'onMalloc',
  'onRealloc', 'onRuntimeInitialized', 'postMainLoop', 'postRun', 'preInit',
  'preMainLoop', 'preRun',
  'preinitializedWebGLContext', 'memoryInitializerRequest', 'preloadPlugins',
  'print', 'printErr', 'quit', 'setStatus', 'statusMessage', 'stderr',
  'stdin', 'stdout', 'thisProgram', 'wasm', 'wasmBinary', 'websocket'
];

// If set to nonzero, the provided virtual filesystem if treated
// case-insensitive, like Windows and macOS do. If set to 0, the VFS is
// case-sensitive, like on Linux.
// [link]
var CASE_INSENSITIVE_FS = 0;

// If set to 0, does not build in any filesystem support. Useful if you are just
// doing pure computation, but not reading files or using any streams (including
// fprintf, and other stdio.h things) or anything related. The one exception is
// there is partial support for printf, and puts, hackishly.  The compiler will
// automatically set this if it detects that syscall usage (which is static)
// does not require a full filesystem. If you still want filesystem support, use
// FORCE_FILESYSTEM
// [link]
var FILESYSTEM = 1;

// Makes full filesystem support be included, even if statically it looks like
// it is not used. For example, if your C code uses no files, but you include
// some JS that does, you might need this.
// [link]
var FORCE_FILESYSTEM = 0;

// Enables support for the NODERAWFS filesystem backend. This is a special
// backend as it replaces all normal filesystem access with direct Node.js
// operations, without the need to do `FS.mount()`, and this backend only
// works with Node.js. The initial working directory will be same as
// process.cwd() instead of VFS root directory.  Because this mode directly uses
// Node.js to access the real local filesystem on your OS, the code will not
// necessarily be portable between OSes - it will be as portable as a Node.js
// program would be, which means that differences in how the underlying OS
// handles permissions and errors and so forth may be noticeable.  This has
// mostly been tested on Linux so far.
// [link]
var NODERAWFS = 0;

// This saves the compiled wasm module in a file with name
//   $WASM_BINARY_NAME.$V8_VERSION.cached
// and loads it on subsequent runs. This caches the compiled wasm code from
// v8 in node, which saves compiling on subsequent runs, making them start up
// much faster.
// The V8 version used in node is included in the cache name so that we don't
// try to load cached code from another version, which fails silently (it seems
// to load ok, but we do actually recompile).
//  * The only version known to work for sure is node 12.9.1, as this has
//    regressed, see
//    https://github.com/nodejs/node/issues/18265#issuecomment-622971547
//  * The default location of the .cached files is alongside the wasm binary,
//    as mentioned earlier. If that is in a read-only directory, you may need
//    to place them elsewhere. You can use the locateFile() hook to do so.
// [link]
var NODE_CODE_CACHING = 0;

// Functions that are explicitly exported. These functions are kept alive
// through LLVM dead code elimination, and also made accessible outside of the
// generated code even after running closure compiler (on "Module").  The
// symbols listed here require an `_` prefix.
//
// By default if this setting is not specified on the command line the
// `_main` function will be implicitly exported.  In STANDALONE_WASM mode the
// default export is `__start` (or `__initialize` if --no-entry is specified).
// [link]
var EXPORTED_FUNCTIONS = [];

// If true, we export all the symbols that are present in JS onto the Module
// object. This does not affect which symbols will be present - it does not
// prevent DCE or cause anything to be included in linking. It only does
//   Module['X'] = X;
// for all X that end up in the JS file. This is useful to export the JS
// library functions on Module, for things like dynamic linking.
// [link]
var EXPORT_ALL = 0;

// Remembers the values of these settings, and makes them accessible
// through Runtime.getCompilerSetting and emscripten_get_compiler_setting.
// To see what is retained, look for compilerSettings in the generated code.
// [link]
var RETAIN_COMPILER_SETTINGS = 0;

// JS library elements (C functions implemented in JS) that we include by
// default. If you want to make sure something is included by the JS compiler,
// add it here.  For example, if you do not use some emscripten_* C API call
// from C, but you want to call it from JS, add it here (and in EXPORTED
// FUNCTIONS with prefix "_", if you use closure compiler).  Note that the name
// may be slightly misleading, as this is for any JS library element, and not
// just functions. For example, you can include the Browser object by adding
// "$Browser" to this list.
// [link]
var DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = [];

// This list is also used to determine auto-exporting of library dependencies
// (i.e., functions that might be dependencies of JS library functions, that if
// so we must export so that if they are implemented in C they will be
// accessible, in ASM_JS mode).
// [link]
var LIBRARY_DEPS_TO_AUTOEXPORT = ['memcpy'];

// Include all JS library functions instead of the sum of
// DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + any functions used by the generated code.
// This is needed when dynamically loading (i.e. dlopen) modules that make use
// of runtime library functions that are not used in the main module.  Note that
// this only applies to js libraries, *not* C. You will need the main file to
// include all needed C libraries.  For example, if a module uses malloc or new,
// you will need to use those in the main file too to pull in malloc for use by
// the module.
// [link]
var INCLUDE_FULL_LIBRARY = 0;

// Set this to a string to override the shell file used
// [link]
var SHELL_FILE = 0;

// If set to 1, we emit relocatable code from the LLVM backend; both
// globals and function pointers are all offset (by gb and fp, respectively)
// Automatically set for SIDE_MODULE or MAIN_MODULE.
// [compile+link]
var RELOCATABLE = 0;

// A main module is a file compiled in a way that allows us to link it to
// a side module at runtime.
//  1: Normal main module.
//  2: DCE'd main module. We eliminate dead code normally. If a side
//     module needs something from main, it is up to you to make sure
//     it is kept alive.
// [compile+link]
var MAIN_MODULE = 0;

// Corresponds to MAIN_MODULE (also supports modes 1 and 2)
// [compile+link]
var SIDE_MODULE = 0;

// If this is a shared object (MAIN_MODULE == 1 || SIDE_MODULE == 1), then we
// will link these at runtime. They must have been built with SIDE_MODULE == 1.
// [link]
var RUNTIME_LINKED_LIBS = [];

// If set to 1, this is a worker library, a special kind of library that is run
// in a worker. See emscripten.h
// [link]
var BUILD_AS_WORKER = 0;

// If set to 1, we build the project into a js file that will run in a worker,
// and generate an html file that proxies input and output to/from it.
// [link]
var PROXY_TO_WORKER = 0;

// If set, the script file name the main thread loads.  Useful if your project
// doesn't run the main emscripten- generated script immediately but does some
// setup before
// [link]
var PROXY_TO_WORKER_FILENAME = '';

// If set to 1, compiles in a small stub main() in between the real main() which
// calls pthread_create() to run the application main() in a pthread.  This is
// something that applications can do manually as well if they wish, this option
// is provided as convenience.
//
// The pthread that main() runs on is a normal pthread in all ways, with the one
// difference that its stack size is the same as the main thread would normally
// have, that is, TOTAL_STACK. This makes it easy to flip between
// PROXY_TO_PTHREAD and non-PROXY_TO_PTHREAD modes with main() always getting
// the same amount of stack.
//
// This proxies Module['canvas'], if present, and if OFFSCREENCANVAS_SUPPORT
// is enabled. This has to happen because this is the only chance - this browser
// main thread does the only pthread_create call that happens on
// that thread, so it's the only chance to transfer the canvas from there.
// [link]
var PROXY_TO_PTHREAD = 0;

// If set to 1, this file can be linked with others, either as a shared library
// or as the main file that calls a shared library. To enable that, we will not
// internalize all symbols and cull the unused ones, in other words, we will not
// remove unused functions and globals, which might be used by another module we
// are linked with.
//
// MAIN_MODULE and SIDE_MODULE both imply this, so it not normally necessary
// to set this explicitly. Note that MAIN_MODULE and SIDE_MODULE mode 2 do
// *not* set this, so that we still do normal DCE on them, and in that case
// you must keep relevant things alive yourself using exporting.
// [link]
var LINKABLE = 0;

// Emscripten 'strict' build mode: Drop supporting any deprecated build options.
// Set the environment variable EMCC_STRICT=1 or pass -s STRICT=1 to test that a
// codebase builds nicely in forward compatible manner.
// Changes enabled by this:
//   * The C define EMSCRIPTEN is not defined (__EMSCRIPTEN__ always is, and
//     is the correct thing to use).
//   * STRICT_JS is enabled.
//   * IGNORE_MISSING_MAIN is disabled.
//   * AUTO_JS_LIBRARIES is disabled.
//   * AUTO_NATIVE_LIBRARIES is disabled.
//   * AUTO_ARCHIVE_INDEXES is disabled.
//   * DEFAULT_TO_CXX is disabled.
// [compile+link]
var STRICT = 0;

// Allow program to link with or without `main` symbol.
// If this is disabled then one must provide a `main` symbol or explicitly
// opt out by passing `--no-entry` or an EXPORTED_FUNCTIONS list that doesn't
// include `_main`.
// [link]
var IGNORE_MISSING_MAIN = 1;

// Automatically attempt to add archive indexes at link time to archives that
// don't already have them.  This can happen when GNU ar or GNU ranlib is used
// rather than `llvm-ar` or `emar` since the former don't understand the wasm
// object format.
// [link]
var AUTO_ARCHIVE_INDEXES = 1;

// Add "use strict;" to generated JS
// [link]
var STRICT_JS = 0;

// If set to 1, we will warn on any undefined symbols that are not resolved by
// the library_*.js files. Note that it is common in large projects to not
// implement everything, when you know what is not going to actually be called
// (and don't want to mess with the existing buildsystem), and functions might
// be implemented later on, say in --pre-js, so you may want to build with -s
// WARN_ON_UNDEFINED_SYMBOLS=0 to disable the warnings if they annoy you.  See
// also ERROR_ON_UNDEFINED_SYMBOLS.  Any undefined symbols that are listed in-
// EXPORTED_FUNCTIONS will also be reported.
// [link]
var WARN_ON_UNDEFINED_SYMBOLS = 1;

// If set to 1, we will give a link-time error on any undefined symbols (see
// WARN_ON_UNDEFINED_SYMBOLS). To allow undefined symbols at link time set this
// to 0, in which case if an undefined function is called a runtime error will
// occur.  Any undefined symbols that are listed in EXPORTED_FUNCTIONS will also
// be reported.
// [link]
var ERROR_ON_UNDEFINED_SYMBOLS = 1;

// Use small chunk size for binary synchronous XHR's in Web Workers.  Used for
// testing.  See test_chunked_synchronous_xhr in runner.py and library.js.
// [link]
var SMALL_XHR_CHUNKS = 0;

// If 1, will include shim code that tries to 'fake' a browser environment, in
// order to let you run a browser program (say, using SDL) in the shell.
// Obviously nothing is rendered, but this can be useful for benchmarking and
// debugging if actual rendering is not the issue. Note that the shim code is
// very partial - it is hard to fake a whole browser! - so keep your
// expectations low for this to work.
// [link]
var HEADLESS = 0;

// If 1, we force Date.now(), Math.random, etc. to return deterministic results.
// This also tries to make execution deterministic across machines and
// environments, for example, not doing anything different based on the
// browser's language setting (which would mean you can get different results
// in different browsers, or in the browser and in node).
// Good for comparing builds for debugging purposes (and nothing else).
// [link]
var DETERMINISTIC = 0;

// By default we emit all code in a straightforward way into the output
// .js file. That means that if you load that in a script tag in a web
// page, it will use the global scope. With `MODULARIZE` set, we instead emit
// the code wrapped in a function that returns a promise. The promise is
// resolved with the module instance when it is safe to run the compiled code,
// similar to the `onRuntimeInitialized` callback. You do not need to use the
// `onRuntimeInitialized` callback when using `MODULARIZE`.
//
// (If WASM_ASYNC_COMPILATION is off, that is, if compilation is
// *synchronous*, then it would not make sense to return a Promise, and instead
// the Module object itself is returned, which is ready to be used.)
//
// The default name of the function is `Module`, but can be changed using the
// `EXPORT_NAME` option. We recommend renaming it to a more typical name for a
// factory function, e.g. `createModule`.
//
//
// You use the factory function like so:
//
//   const module = await EXPORT_NAME();
//
// or:
//
//   let module;
//   EXPORT_NAME().then(instance => {
//     module = instance;
//   });
//
//
// The factory function accepts 1 parameter, an object with default values for
// the module instance:
//
//   const module = await EXPORT_NAME({ option: value, ... });
//
// Note the parentheses - we are calling EXPORT_NAME in order to instantiate
// the module. This allows you to create multiple instances of the module.
//
// Note that in MODULARIZE mode we do *not* look for a global `Module` object
// for default values. Default values must be passed as a parameter to the
// factory function.
//
// The default .html shell file provided in MINIMAL_RUNTIME mode will create
// a singleton instance automatically, to run the application on the page.
// (Note that it does so without using the Promise API mentioned earlier, and
// so code for the Promise is not even emitted in the .js file if you tell
// emcc to emit an .html output.)
// The default .html shell file provided by traditional runtime mode is only
// compatible with MODULARIZE=0 mode, so when building with traditional
// runtime, you should provided your own html shell file to perform the
// instantiation when building with MODULARIZE=1. (For more details, see
// https://github.com/emscripten-core/emscripten/issues/7950)
//
// If you add --pre-js or --post-js files, they will be included inside
// the factory function with the rest of the emitted code in order to be
// optimized together with it.
//
// If you want to include code outside all of the generated code, including the
// factory function, you can use --extern-pre-js or --extern-post-js. While
// --pre-js and --post-js happen to do that in non-MODULARIZE mode, their
// intended usage is to add code that is optimized with the rest of the emitted
// code, allowing better dead code elimination and minification.
// [link]
var MODULARIZE = 0;

// Export using an ES6 Module export rather than a UMD export.  MODULARIZE must
// be enabled for ES6 exports.
// [link]
var EXPORT_ES6 = 0;

// Use the ES6 Module relative import feature 'import.meta.url'
// to auto-detect WASM Module path.
// It might not be supported on old browsers / toolchains
// [link]
var USE_ES6_IMPORT_META = 1;

// If 1, will just time how long main() takes to execute, and not print out
// anything at all whatsoever. This is useful for benchmarking.
// [link]
var BENCHMARK = 0;

// Global variable to export the module as for environments without a
// standardized module loading system (e.g. the browser and SM shell).
// [link]
var EXPORT_NAME = 'Module';

// When set to 0, we do not emit eval() and new Function(), which disables some functionality
// (causing runtime errors if attempted to be used), but allows the emitted code to be
// acceptable in places that disallow dynamic code execution (chrome packaged app,
// privileged firefox app, etc.). Pass this flag when developing an Emscripten application
// that is targeting a privileged or a certified execution environment, see
// Firefox Content Security Policy (CSP) webpage for details:
// https://developer.mozilla.org/en-US/Apps/Build/Building_apps_for_Firefox_OS/CSP
// When this flag is set, the following features (linker flags) are unavailable:
//  --closure 1: When using closure compiler, eval() would be needed to locate the Module object.
//  -s RELOCATABLE=1: the function Runtime.loadDynamicLibrary would need to eval().
//  --bind: Embind would need to eval().
// Additionally, the following Emscripten runtime functions are unavailable when
// DYNAMIC_EXECUTION=0 is set, and an attempt to call them will throw an exception:
// - emscripten_run_script(),
// - emscripten_run_script_int(),
// - emscripten_run_script_string(),
// - dlopen(),
// - the functions ccall() and cwrap() are still available, but they are restricted to only
//   being able to call functions that have been exported in the Module object in advance.
// When set to -s DYNAMIC_EXECUTION=2 flag is set, attempts to call to eval() are demoted
// to warnings instead of throwing an exception.
// [link]
var DYNAMIC_EXECUTION = 1;

// whether we are in the generate struct_info bootstrap phase
// [link]
var BOOTSTRAPPING_STRUCT_INFO = 0;

// Add some calls to emscripten tracing APIs
// [compile+link]
var EMSCRIPTEN_TRACING = 0;

// Specify the GLFW version that is being linked against.  Only relevant, if you
// are linking against the GLFW library.  Valid options are 2 for GLFW2 and 3
// for GLFW3.
// [link]
var USE_GLFW = 2;

// Whether to use compile code to WebAssembly. Set this to 0 to compile to JS
// instead of wasm.
//
// Specify -s WASM=2 to target both WebAssembly and JavaScript at the same time.
// In that build mode, two files a.wasm and a.wasm.js are produced, and at runtime
// the WebAssembly file is loaded if browser/shell supports it. Otherwise the
// .wasm.js fallback will be used.
//
// If WASM=2 is enabled and the browser fails to compile the WebAssembly module,
// the page will be reloaded in Wasm2JS mode.
// [link]
var WASM = 1;

// STANDALONE_WASM indicates that we want to emit a wasm file that can run
// without JavaScript. The file will use standard APIs such as wasi as much as
// possible to achieve that.
//
// This option does not guarantee that the wasm can be used by itself - if you
// use APIs with no non-JS alternative, we will still use those (e.g., OpenGL
// at the time of writing this). This gives you the option to see which APIs
// are missing, and if you are compiling for a custom wasi embedding, to add
// those to your embedding.
//
// We may still emit JS with this flag, but the JS should only be a convenient
// way to run the wasm on the Web or in Node.js, and you can run the wasm by
// itself without that JS (again, unless you use APIs for which there is no
// non-JS alternative) in a wasm runtime like wasmer or wasmtime.
//
// Note that even without this option we try to use wasi etc. syscalls as much
// as possible. What this option changes is that we do so even when it means
// a tradeoff with JS size. For example, when this option is set we do not
// import the Memory - importing it is useful for JS, so that JS can start to
// use it before the wasm is even loaded, but in wasi and other wasm-only
// environments the expectation is to create the memory in the wasm itself.
// Doing so prevents some possible JS optimizations, so we only do it behind
// this flag.
//
// When this flag is set we do not legalize the JS interface, since the wasm is
// meant to run in a wasm VM, which can handle i64s directly. If we legalized it
// the wasm VM would not recognize the API. However, this means that the
// optional JS emitted won't run if you use a JS API with an i64. You can use
// the WASM_BIGINT option to avoid that problem by using BigInts for i64s which
// means we don't need to legalize for JS (but this requires a new enough JS
// VM).
//
// Standlone builds require a `main` entry point by default.  If you want to
// build a library (also known as a reactor) instead you can pass `--no-entry`.
// [link]
var STANDALONE_WASM = 0;

// Whether to ignore implicit traps when optimizing in binaryen.  Implicit
// traps are the traps that happen in a load that is out of bounds, or
// div/rem of 0, etc. With this option set, the optimizer assumes that loads
// cannot trap, and therefore that they have no side effects at all. This
// is *not* safe in general, as you may have a load behind a condition which
// ensures it it is safe; but if the load is assumed to not have side effects it
// could be executed unconditionally. For that reason this option is generally
// not useful on large and complex projects, but in a small and simple enough
// codebase it may help reduce code size a little bit.
// [link]
var BINARYEN_IGNORE_IMPLICIT_TRAPS = 0;

// A comma-separated list of extra passes to run in the binaryen optimizer,
// Setting this does not override/replace the default passes. It is appended at
// the end of the list of passes.
// [link]
var BINARYEN_EXTRA_PASSES = "";

// Whether to compile the wasm asynchronously, which is more efficient and does
// not block the main thread. This is currently required for all but the
// smallest modules to run in chrome.
//
// (This option was formerly called BINARYEN_ASYNC_COMPILATION)
// [link]
var WASM_ASYNC_COMPILATION = 1;

// If set to 1, the dynCall() and dynCall_sig() API is made available
// to caller.
// [link]
var DYNCALLS = 0;

// WebAssembly integration with JavaScript BigInt. When enabled we don't need
// to legalize i64s into pairs of i32s, as the wasm VM will use a BigInt where
// an i64 is used.
// [link]
var WASM_BIGINT = 0;

// WebAssembly defines a "producers section" which compilers and tools can
// annotate themselves in, and LLVM emits this by default.
// Emscripten will strip that out so that it is *not* emitted because it
// increases code size, and also some users may not want information
// about their tools to be included in their builds for privacy or security
// reasons, see
// https://github.com/WebAssembly/tool-conventions/issues/93.
// [link]
var EMIT_PRODUCERS_SECTION = 0;

// If set then generated WASM files will contain a custom
// "emscripten_metadata" section that contains information necessary
// to execute the file without the accompanying JS file.
// [link]
var EMIT_EMSCRIPTEN_METADATA = 0;

// Emits emscripten license info in the JS output.
// [link]
var EMIT_EMSCRIPTEN_LICENSE = 0;

// Whether to legalize the JS FFI interfaces (imports/exports) by wrapping them
// to automatically demote i64 to i32 and promote f32 to f64. This is necessary
// in order to interface with JavaScript.  For non-web/non-JS embeddings, setting
// this to 0 may be desirable.
// [link]
var LEGALIZE_JS_FFI = 1;

// Ports

// Specify the SDL version that is being linked against.
// 1, the default, is 1.3, which is implemented in JS
// 2 is a port of the SDL C code on emscripten-ports
// [link]
var USE_SDL = 1;

// Specify the SDL_gfx version that is being linked against. Must match USE_SDL
// [link]
var USE_SDL_GFX = 0;

// Specify the SDL_image version that is being linked against. Must match USE_SDL
// [link]
var USE_SDL_IMAGE = 1;

// Specify the SDL_ttf version that is being linked against. Must match USE_SDL
// [link]
var USE_SDL_TTF = 1;

// Specify the SDL_net version that is being linked against. Must match USE_SDL
// [link]
var USE_SDL_NET = 1;

// 1 = use icu from emscripten-ports
// [link]
var USE_ICU = 0;

// 1 = use zlib from emscripten-ports
// [link]
var USE_ZLIB = 0;

// 1 = use bzip2 from emscripten-ports
// [link]
var USE_BZIP2 = 0;

// 1 = use giflib from emscripten-ports
// [link]
var USE_GIFLIB = 0;

// 1 = use libjpeg from emscripten-ports
// [link]
var USE_LIBJPEG = 0;

// 1 = use libpng from emscripten-ports
// [link]
var USE_LIBPNG = 0;

// 1 = use Regal from emscripten-ports
// [link]
var USE_REGAL = 0;

// 1 = use Boost headers from emscripten-ports
// [link]
var USE_BOOST_HEADERS = 0;

// 1 = use bullet from emscripten-ports
// [link]
var USE_BULLET = 0;

// 1 = use vorbis from emscripten-ports
// [link]
var USE_VORBIS = 0;

// 1 = use ogg from emscripten-ports
// [link]
var USE_OGG = 0;

// 1 = use mpg123 from emscripten-ports
// [link]
var USE_MPG123 = 0;

// 1 = use freetype from emscripten-ports
// [link]
var USE_FREETYPE = 0;

// Specify the SDL_mixer version that is being linked against.
// Doesn't *have* to match USE_SDL, but a good idea.
// [link]
var USE_SDL_MIXER = 1;

// 1 = use harfbuzz from harfbuzz upstream
// [link]
var USE_HARFBUZZ = 0;

// 3 = use cocos2d v3 from emscripten-ports
// [link]
var USE_COCOS2D = 0;

// 1 = use libmodplug from emscripten-ports
// [link]
var USE_MODPLUG = 0;

// Formats to support in SDL2_image. Valid values: bmp, gif, lbm, pcx, png, pnm, tga, xcf, xpm, xv
// [link]
var SDL2_IMAGE_FORMATS = [];

// Formats to support in SDL2_mixer. Valid values: ogg, mp3
// [link]
var SDL2_MIXER_FORMATS = ["ogg"];

// The list of defines (C_DEFINES) was moved into struct_info.json in the same
// directory.  That file is automatically parsed by tools/gen_struct_info.py.
// If you modify the headers, just clear your cache and emscripten libc should
// see the new values.

// If true, the current build is performed for the Emscripten test harness.
// [other]
var IN_TEST_HARNESS = 0;

// If true, enables support for pthreads.
// This setting is equivalent to `-pthread`, which should be preferred.
// [compile+link] - affects user code at compile and system libraries at link.
var USE_PTHREADS = 0;

// In web browsers, Workers cannot be created while the main browser thread
// is executing JS/Wasm code, but the main thread must regularly yield back
// to the browser event loop for Worker initialization to occur.
// This means that pthread_create() is essentially an asynchronous operation
// when called from the main browser thread, and the main thread must
// repeatedly yield back to the JS event loop in order for the thread to
// actually start.
// If your application needs to be able to synchronously create new threads,
// you can pre-create a pthread pool by specifying -s PTHREAD_POOL_SIZE=x,
// in which case the specified number of Workers will be preloaded into a pool
// before the application starts, and that many threads can then be available
// for synchronous creation.
// Note that this setting is a string, and will be emitted in the JS code
// (directly, with no extra quotes) so that if you set it to '5' then 5 workers
// will be used in the pool, and so forth. The benefit of this being a string
// is that you can set it to something like
// 'navigator.hardwareConcurrency' (which will use the number of cores the
// browser reports, and is how you can get exactly enough workers for a
// threadpool equal to the number of cores).
// [link] - affects generated JS runtime code at link time
var PTHREAD_POOL_SIZE = '';

// Normally, applications can create new threads even when the pool is empty.
// When application breaks out to the JS event loop before trying to block on
// the thread via `pthread_join` or any other blocking primitive,
// an extra Worker will be created and the thread callback will be executed.
// However, breaking out to the event loop requires custom modifications to
// the code to adapt it to the Web, and not something that works for
// off-the-shelf apps. Those apps without any modifications are most likely
// to deadlock. This setting ensures that, instead of a risking a deadlock,
// they get a runtime EAGAIN error instead that can at least be gracefully
// handled from the C / C++ side.
// Values:
//  - `0` - disable warnings on thread pool exhaustion
//  - `1` - enable warnings on thread pool exhaustion (default)
//  - `2` - make thread pool exhaustion a hard error
// [link]
var PTHREAD_POOL_SIZE_STRICT = 1;

// If your application does not need the ability to synchronously create
// threads, but it would still like to opportunistically speed up initial thread
// startup time by prewarming a pool of Workers, you can specify the size of
// the pool with -s PTHREAD_POOL_SIZE=x, but then also specify
// -s PTHREAD_POOL_DELAY_LOAD=1, which will cause the runtime to not wait up at
// startup for the Worker pool to finish loading. Instead, the runtime will
// immediately start up and the Worker pool will asynchronously spin up in
// parallel on the background. This can shorten the time that pthread_create()
// calls take to actually start a thread, but without actually slowing down
// main application startup speed. If PTHREAD_POOL_DELAY_LOAD=0 (default),
// then the runtime will wait for the pool to start up before running main().
// [link] - affects generated JS runtime code at link time
var PTHREAD_POOL_DELAY_LOAD = 0;

// If not explicitly specified, this is the stack size to use for newly created
// pthreads.  According to
// http://man7.org/linux/man-pages/man3/pthread_create.3.html, default stack
// size on Linux/x86-32 for a new thread is 2 megabytes, so follow the same
// convention. Use pthread_attr_setstacksize() at thread creation time to
// explicitly specify the stack size, in which case this value is ignored. Note
// that the wasm function call control flow stack is separate from this
// stack, and this stack only contains certain function local variables, such as
// those that have their addresses taken, or ones that are too large to fit as
// local vars in wasm code.
// [link]
var DEFAULT_PTHREAD_STACK_SIZE = 2*1024*1024;

// True when building with --threadprofiler
// [link]
var PTHREADS_PROFILING = 0;

// It is dangerous to call pthread_join or pthread_cond_wait
// on the main thread, as doing so can cause deadlocks on the Web (and also
// it works using a busy-wait which is expensive). See
// https://emscripten.org/docs/porting/pthreads.html#blocking-on-the-main-browser-thread
// This may become set to 0 by default in the future; for now, this just
// warns in the console.
// [link]
var ALLOW_BLOCKING_ON_MAIN_THREAD = 1;

// If true, add in debug traces for diagnosing pthreads related issues.
// [link]
var PTHREADS_DEBUG = 0;

// This tries to evaluate global ctors at compile-time, applying their effects
// into the mem init file. This saves running code during startup, and also
// allows removing the global ctor functions and other code that only they used,
// so this is also good for reducing code size. However, this does make the
// compile step much slower.
//
// This basically runs the ctors during compile time, seeing if they execute
// safely in a sandbox. Any ffi access out of wasm causes failure, as it could
// do something nondeterministic and/or alter some other state we don't see. If
// all the global ctor does is pure computation inside wasm, it should be ok.
// Run with EMCC_DEBUG=1 in the env to see logging, and errors when it fails to
// eval (you'll see a message, or a stack trace; in the latter case, the
// functions on the stack should give you an idea of what ffi was called and
// why, and perhaps you can refactor your code to avoid it, e.g., remove
// mallocs, printfs in global ctors).
//
// This optimization can increase the size of the mem init file, because ctors
// can write to memory that would otherwise be in a zeroinit area. This may not
// be a significant increase after gzip, if there are mostly zeros in there, and
// in any case the mem init increase would be offset by a code size decrease.
// (Unless you have a small ctor that writes 'random' data to memory, which
// would reduce little code but add potentially lots of uncompressible data.)
//
// LLVM's GlobalOpt *almost* does this operation. It does in simple cases, where
// LLVM IR is not too complex for its logic to evaluate, but it isn't powerful
// enough for e.g. libc++ iostream ctors. It is just hard to do at the LLVM IR
// level - LLVM IR is complex and getting more complex, this would require
// GlobalOpt to have a full interpreter, plus a way to write back into LLVM IR
// global objects.  At the wasm level, however, everything has been lowered
// into a simple low level, and we also just need to write bytes into an array,
// so this is easy for us to do, but not for LLVM. A further issue for LLVM is
// that it doesn't know that we will not link in further code, so it only tries
// to optimize ctors with lowest priority. We do know that, and can optimize all
// the ctors.
// [link]
var EVAL_CTORS = 0;

// Is enabled, use the JavaScript TextDecoder API for string marshalling.
// Enabled by default, set this to 0 to disable.
// If set to 2, we assume TextDecoder is present and usable, and do not emit
// any JS code to fall back if it is missing. In single threaded -Oz build modes,
// TEXTDECODER defaults to value == 2 to save code size.
// [link]
var TEXTDECODER = 1;

// Embind specific: If enabled, assume UTF-8 encoded data in std::string binding.
// Disable this to support binary data transfer.
// [link]
var EMBIND_STD_STRING_IS_UTF8 = 1;

// If set to 1, enables support for transferring canvases to pthreads and
// creating WebGL contexts in them, as well as explicit swap control for GL
// contexts. This needs browser support for the OffscreenCanvas specification.
// [link]
var OFFSCREENCANVAS_SUPPORT = 0;

// If you are using PROXY_TO_PTHREAD with OFFSCREENCANVAS_SUPPORT, then specify
// here a comma separated list of CSS ID selectors to canvases to proxy over
// to the pthread at program startup, e.g. '#canvas1, #canvas2'.
// [link]
var OFFSCREENCANVASES_TO_PTHREAD = "#canvas";

// If set to 1, enables support for WebGL contexts to render to an offscreen
// render target, to avoid the implicit swap behavior of WebGL where exiting any
// event callback would automatically perform a "flip" to present rendered
// content on screen. When an Emscripten GL context has Offscreen Framebuffer
// enabled, a single frame can be composited from multiple event callbacks, and
// the swap function emscripten_webgl_commit_frame() is then explicitly called
// to present the rendered content on screen.
//
// The OffscreenCanvas feature also enables explicit GL frame swapping support,
// and also, -s OFFSCREEN_FRAMEBUFFER=1 feature can be used to polyfill support
// for accessing WebGL in multiple threads in the absence of OffscreenCanvas
// support in browser, at the cost of some performance and latency.
// OffscreenCanvas and Offscreen Framebuffer support can be enabled at the same
// time, and allows one to utilize OffscreenCanvas where available, and to fall
// back to Offscreen Framebuffer otherwise.
// [link]
var OFFSCREEN_FRAMEBUFFER = 0;

// If nonzero, Fetch API (and hence ASMFS) supports backing to IndexedDB. If 0, IndexedDB is not utilized. Set to 0 if
// IndexedDB support is not interesting for target application, to save a few kBytes.
// [link]
var FETCH_SUPPORT_INDEXEDDB = 1;

// If nonzero, prints out debugging information in library_fetch.js
// [link]
var FETCH_DEBUG = 0;

// If nonzero, enables emscripten_fetch API.
// [link]
var FETCH = 0;

// If set to 1, uses the multithreaded filesystem that is implemented within the
// wasm module, using emscripten_fetch. Implies -s FETCH=1.
// [link]
var ASMFS = 0;

// If set to 1, embeds all subresources in the emitted file as base64 string
// literals. Embedded subresources may include (but aren't limited to) wasm,
// asm.js, and static memory initialization code.
//
// When using code that depends on this option, your Content Security Policy may
// need to be updated. Specifically, embedding asm.js requires the script-src
// directive to allow 'unsafe-inline', and using a Worker requires the
// child-src directive to allow blob:. If you aren't using Content Security
// Policy, or your CSP header doesn't include either script-src or child-src,
// then you can safely ignore this warning.
// [link]
var SINGLE_FILE = 0;

// If set to 1, all JS libraries will be automatically available at link time.
// This gets set to 0 in STRICT mode (or with MINIMAL_RUNTIME) which mean you
// need to explicitly specify -lfoo.js in at link time in order to access
// library function in library_foo.js.
// [link]
var AUTO_JS_LIBRARIES = 1;

// Like AUTO_JS_LIBRARIES but for the native libraries such as libgl, libal
// and libhtml5.   If this is disabled it is necessary to explcitly add
// e.g. -lhtml5 and also to first build the library using `embuilder`.
// [link]
var AUTO_NATIVE_LIBRARIES = 1;

// Specifies the oldest major version of Firefox to target. I.e. all Firefox
// versions >= MIN_FIREFOX_VERSION
// are desired to work. Pass -s MIN_FIREFOX_VERSION=majorVersion to drop support
// for Firefox versions older than < majorVersion.
// Firefox ESR 60.5 (Firefox 65) was released on 2019-01-29.
// [link]
var MIN_FIREFOX_VERSION = 65;

// Specifies the oldest version of desktop Safari to target. Version is encoded
// in MMmmVV, e.g. 70101 denotes Safari 7.1.1.
// Safari 12.0.0 was released on September 17, 2018, bundled with macOS 10.14.0
// Mojave.
// NOTE: Emscripten is unable to produce code that would work in iOS 9.3.5 and
// older, i.e. iPhone 4s, iPad 2, iPad 3, iPad Mini 1, Pod Touch 5 and older,
// see https://github.com/emscripten-core/emscripten/pull/7191.
// [link]
var MIN_SAFARI_VERSION = 120000;

// Specifies the oldest version of Internet Explorer to target. E.g. pass -s
// MIN_IE_VERSION = 11 to drop support for IE 10 and older.
// Internet Explorer is at end of life and does not support WebAssembly.
// MAX_INT (0x7FFFFFFF) specifies that target is not supported.
// [link]
var MIN_IE_VERSION = 0x7FFFFFFF;

// Specifies the oldest version of Edge (EdgeHTML, the non-Chromium based
// flavor) to target. E.g. pass -s MIN_EDGE_VERSION=40 to drop support for
// EdgeHTML 39 and older.
// Edge 44.17763 was released on November 13, 2018
// [link]
var MIN_EDGE_VERSION = 44;

// Specifies the oldest version of Chrome. E.g. pass -s MIN_CHROME_VERSION=58 to
// drop support for Chrome 57 and older.
// Chrome 75.0.3770 was released on 2019-06-04
// [link]
var MIN_CHROME_VERSION = 75;

// Tracks whether we are building with errno support enabled. Set to 0
// to disable compiling errno support in altogether. This saves a little
// bit of generated code size in applications that do not care about
// POSIX errno variable. Setting this to 0 also requires using --closure
// for effective code size optimizations to take place.
// [link]
var SUPPORT_ERRNO = 1;

// If true, uses minimal sized runtime without POSIX features, Module,
// preRun/preInit/etc., Emscripten built-in XHR loading or library_browser.js.
// Enable this setting to target the smallest code size possible.  Set
// MINIMAL_RUNTIME=2 to further enable even more code size optimizations. These
// opts are quite hacky, and work around limitations in Closure and other parts
// of the build system, so they may not work in all generated programs (But can
// be useful for really small programs)
// [link]
var MINIMAL_RUNTIME = 0;

// If set to 1, MINIMAL_RUNTIME will utilize streaming WebAssembly compilation,
// where WebAssembly module is compiled already while it is being downloaded.
// In order for this to work, the web server MUST properly serve the .wasm file
// with a HTTP response header "Content-Type: application/wasm". If this HTTP
// header is not present, e.g. Firefox 73 will fail with an error message
//    TypeError: Response has unsupported MIME type
// and Chrome 78 will fail with an error message
//    Uncaught (in promise) TypeError: Failed to execute 'compile' on
//    'WebAssembly': Incorrect response MIME type. Expected 'application/wasm'.
// If set to 0 (default), streaming WebAssembly compilation is disabled, which
// means that the WebAssembly Module will first be downloaded fully, and only
// then compilation starts.
// For large .wasm modules and production environments, this should be set to 1
// for faster startup speeds. However this setting is disabled by default
// since it requires server side configuration and for really small pages there
// is no observable difference (also has a ~100 byte impact to code size)
// [link]
var MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION = 0;

// If set to 1, MINIMAL_RUNTIME will utilize streaming WebAssembly instantiation,
// where WebAssembly module is compiled+instantiated already while it is being
// downloaded. Same restrictions/requirements apply as with
// MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION.
// MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION and
// MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION cannot be simultaneously active.
// Which one of these two is faster depends on the size of the wasm module,
// the size of the JS runtime file, and the size of the preloaded data file
// to download, and the browser in question.
// [link]
var MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION = 0;

// If building with MINIMAL_RUNTIME=1 and application uses sbrk()/malloc(),
// enable this. If you are not using dynamic allocations, can set this to 0 to
// save code size. This setting is ignored when building with -s
// MINIMAL_RUNTIME=0.
// [link]
var USES_DYNAMIC_ALLOC = 1;

// Advanced manual dead code elimination: Specifies the set of runtime JS
// functions that should be imported to the wasm module.  Remove elements
// from this list to make build smaller if some of these are not needed.  In
// -O3/-Os builds, adjusting this is not necessary, as the Meta-DCE pass is
// able to remove these, but if you doing a -O2 build or lower, then this can be
// beneficial.
// [link]
var RUNTIME_FUNCS_TO_IMPORT = ['abort', 'setTempRet0', 'getTempRet0']

// If true, compiler supports setjmp() and longjmp(). If false, these APIs are
// not available.  If you are using C++ exceptions, but do not need
// setjmp()+longjmp() API, then you can set this to 0 to save a little bit of
// code size and performance when catching exceptions.
// [compile+link] - at compile time this enables the transformations needed for
// longjmp support at codegen time, while at link it allows linking in the
// library support.
// [link]
var SUPPORT_LONGJMP = 1;

// If set to 1, disables old deprecated HTML5 API event target lookup behavior.
// When enabled, there is no "Module.canvas" object, no magic "null" default
// handling, and DOM element 'target' parameters are taken to refer to CSS
// selectors, instead of referring to DOM IDs.
// [link]
var DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR = 1;

// Certain browser DOM API operations, such as requesting fullscreen mode
// transition or pointer lock require that the request originates from within
// an user initiated event, such as mouse click or keyboard press. Refactoring
// an application to follow this kind of program structure can be difficult, so
// HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS=1 flag allows transparent
// emulation of this by deferring synchronous fullscreen mode and pointer lock
// requests until a suitable event callback is generated. Set this to 0
// to disable support for deferring to save code space if your application does
// not need support for deferred calls.
// [link]
var HTML5_SUPPORT_DEFERRING_USER_SENSITIVE_REQUESTS = 1;

// Specifies whether the generated .html file is run through html-minifier. The
// set of optimization passes run by html-minifier depends on debug and
// optimization levels. In -g2 and higher, no minification is performed. In -g1,
// minification is done, but whitespace is retained. Minification requires at
// least -O1 or -Os to be used. Pass -s MINIFY_HTML=0 to explicitly choose to
// disable HTML minification altogether.
// [link]
var MINIFY_HTML = 1;

// Whether we *may* be using wasm2js. This compiles to wasm normally, but lets
// you run wasm2js *later* on the wasm, and you can pick between running the
// normal wasm or that wasm2js code. For details of how to do that, see the
// test_maybe_wasm2js test.  This option can be useful for debugging and
// bisecting.
// [link]
var MAYBE_WASM2JS = 0;

// This option is no longer used. The appropriate shadow memory size is now
// calculated from INITIAL_MEMORY and MAXIMUM_MEMORY. Will be removed in a
// future release.
// [link]
var ASAN_SHADOW_SIZE = -1

// Internal: Tracks whether Emscripten should link in exception throwing (C++
// 'throw') support library. This does not need to be set directly, but pass
// -fno-exceptions to the build disable exceptions support. (This is basically
// -fno-exceptions, but checked at final link time instead of individual .cpp
// file compile time) If the program *does* contain throwing code (some source
// files were not compiled with `-fno-exceptions`), and this flag is set at link
// time, then you will get errors on undefined symbols, as the exception
// throwing code is not linked in. If so you should either unset the option (if
// you do want exceptions) or fix the compilation of the source files so that
// indeed no exceptions are used).
// TODO(sbc): Move to settings_internal (current blocked due to use in test
// code).
// [link]
var DISABLE_EXCEPTION_THROWING = 0;

// Whether we should use the offset converter.  This is needed for older
// versions of v8 (<7.7) that does not give the hex module offset into wasm
// binary in stack traces, as well as for avoiding using source map entries
// across function boundaries.
// [link]
var USE_OFFSET_CONVERTER = 0;

// If set to 1, the JS compiler is run before wasm-ld so that the linker can
// report undefined symbols within the binary.  Without this option that linker
// doesn't know which symbols might be defined JS and so reporting of undefined
// symbols is delayed until the JS compiler is run.
// [link]
var LLD_REPORT_UNDEFINED = 0;

// Default to c++ mode even when run as `emcc` rather then `emc++`.
// When this is disabled `em++` is required when compiling and linking C++
// programs. This which matches the behaviour of gcc/g++ and clang/clang++.
// [compile+link]
var DEFAULT_TO_CXX = 1;

// While LLVM's wasm32 has long double = float128, we don't support printing
// that at full precision by default. Instead we print as 64-bit doubles, which
// saves libc code size. You can flip this option on to get a libc with full
// long double printing precision.
// [link]
var PRINTF_LONG_DOUBLE = 0;

// Run wabt's wasm2c tool on the final wasm, and combine that with a C runtime,
// resulting in a .c file that you can compile with a C compiler to get a
// native executable that works the same as the normal js+wasm. This will also
// emit the wasm2c .h file. The output filenames will be X.wasm.c, X.wasm.h
// if your output is X.js or X.wasm (note the added .wasm. we make sure to emit,
// which avoids trampling a C file).
// [link]
var WASM2C = 0;

// Setting this affects the path emitted in the wasm that refers to the DWARF
// file, in -gseparate-dwarf mode. This allows the debugging file to be hosted
// in a custom location.
// [link]
var SEPARATE_DWARF_URL = '';

// Emscripten runs wasm-ld to link, and in some cases will do further changes to
// the wasm afterwards, like running wasm-opt to optimize the binary in
// optimized builds. However, in some builds no wasm changes are necessary after
// link. This can make the entire link step faster, and can also be important
// for other reasons, like in debugging if the wasm is not modified then the
// DWARF info from LLVM is preserved (wasm-opt can rewrite it in some cases, but
// not in others like split-dwarf).
// When this flag is turned on, we error at link time if the build requires any
// changes to the wasm after link. This can be useful in testing, for example.
// [link]
var ERROR_ON_WASM_CHANGES_AFTER_LINK = 0;

// Whether the program should abort when an unhandled WASM exception is encountered.
// This makes the Emscripten program behave more like a native program where the OS
// would terminate the process and no further code can be executed when an unhandled
// exception (e.g. out-of-bounds memory access) happens.
// This will instrument all exported functions to catch thrown exceptions and
// call abort() when they happen. Once the program aborts any exported function calls
// will fail with a "program has already aborted" exception to prevent calls into
// code with a potentially corrupted program state.
// This adds a small fixed amount to code size in optimized builds and a slight overhead
// for the extra instrumented function indirection.
// Enable this if you want Emscripten to handle unhandled exceptions nicely at the
// cost of a few bytes extra.
// [link]
var ABORT_ON_WASM_EXCEPTIONS = 0;

// Build binaries that use as many WASI APIs as possible, and include additional
// JS support libraries for those APIs.  This allows emscripten to produce binaries
// are more WASI compilant and also allows it to process and execute WASI
// binaries built with other SDKs (e.g.  wasi-sdk).
// This setting is experimental and subject to change or removal.
// Implies STANDALONE_WASM.
// [link]
var PURE_WASI = 0;

// Set to 1 to define the WebAssembly.Memory object outside of the wasm
// module.  By default the wasm module defines the memory and exports
// it to JavaScript.
// Use of the following settings will enable this settings since they
// depend on being able to define the memory in JavaScript:
// - USE_PTHREADS
// - RELOCATABLE
// - ASYNCIFY_LAZY_LOAD_CODE
// - WASM2JS (WASM=0)
// [link]
var IMPORTED_MEMORY = 0;

// Generate code to loading split wasm modules.
// This option will automatically generate two wasm files as output, one
// with the `.orig` suffix and one without.  The default file (without
// the suffix) when run will generate instrumentation data can later be
// fed into wasm-split (the binaryen tool).
// As well as this the generated JS code will contains help functions
// to loading split modules.
// [link]
var SPLIT_MODULE = 0;

// How to calculate reverse dependencies (dependencies from JS functions to
// native functions) prior to linking native code with wasm-ld.  This option
// has three possible values:
// 'auto': (default) inspect the object code passed to the linker (by running
//         llvm-nm) all all inputs and use the map in deps_info.py to
//         determines the set of additional dependencies.
// 'all' : Include the full set of possible reverse dependencies.
// 'none': No reverse dependences will be added by emscriopten. Any reverse
//         dependencies will be assumed to be explicitly add to
//         EXPORTED_FUNCTIONS and deps_info.py will be completely ignored.
// While 'auto' will produce a minimal set (so is good for code size), 'all'
// and 'none' will give faster link times, especially for very large projects
// (since they both avoid the running of llvm-nm on all linker inputs).
// [link]
var REVERSE_DEPS = 'auto';

//===========================================
// Internal, used for testing only, from here
//===========================================

// Internal (testing only): Disables the blitOffscreenFramebuffer VAO path.
// [link]
var OFFSCREEN_FRAMEBUFFER_FORBID_VAO_PATH = 0;

// Internal (testing only): Forces memory growing to fail.
// [link]
var TEST_MEMORY_GROWTH_FAILS = 0;

// For renamed settings the format is:
// [OLD_NAME, NEW_NAME]
// For removed settings (which now effectively have a fixed value and can no
// longer be changed) the format is:
// [OPTION_NAME, POSSIBLE_VALUES, ERROR_EXPLANATION], where POSSIBLE_VALUES is
// an array of values that will still be silently accepted by the compiler.
// First element in the list is the canonical/fixed value going forward.
// This allows existing build systems to keep specifying one of the supported
// settings, for backwards compatibility.
// When a setting has been removed, and we want to error on all values of it,
// we can set POSSIBLE_VALUES to an impossible value (like "disallowed" for a
// numeric setting, or -1 for a string setting).
var LEGACY_SETTINGS = [
  ['BINARYEN', 'WASM'],
  ['BINARYEN_ASYNC_COMPILATION', 'WASM_ASYNC_COMPILATION'],
  ['UNALIGNED_MEMORY', [0], 'forced unaligned memory not supported in fastcomp'],
  ['FORCE_ALIGNED_MEMORY', [0], 'forced aligned memory is not supported in fastcomp'],
  ['PGO', [0], 'pgo no longer supported'],
  ['QUANTUM_SIZE', [4], 'altering the QUANTUM_SIZE is not supported'],
  ['FUNCTION_POINTER_ALIGNMENT', [2], 'Starting from Emscripten 1.37.29, no longer available (https://github.com/emscripten-core/emscripten/pull/6091)'],
  // Reserving function pointers is not needed - allowing table growth allows any number of new functions to be added.
  ['RESERVED_FUNCTION_POINTERS', 'ALLOW_TABLE_GROWTH'],
  ['BUILD_AS_SHARED_LIB', [0], 'Starting from Emscripten 1.38.16, no longer available (https://github.com/emscripten-core/emscripten/pull/7433)'],
  ['SAFE_SPLIT_MEMORY', [0], 'Starting from Emscripten 1.38.19, SAFE_SPLIT_MEMORY codegen is no longer available (https://github.com/emscripten-core/emscripten/pull/7465)'],
  ['SPLIT_MEMORY', [0], 'Starting from Emscripten 1.38.19, SPLIT_MEMORY codegen is no longer available (https://github.com/emscripten-core/emscripten/pull/7465)'],
  ['BINARYEN_METHOD', ['native-wasm'], 'Starting from Emscripten 1.38.23, Emscripten now always builds either to Wasm (-s WASM=1 - default), or to JavaScript (-s WASM=0), other methods are not supported (https://github.com/emscripten-core/emscripten/pull/7836)'],
  ['BINARYEN_TRAP_MODE', [-1], 'The wasm backend does not support a trap mode (it always clamps, in effect)'],
  ['PRECISE_I64_MATH', [1, 2], 'Starting from Emscripten 1.38.26, PRECISE_I64_MATH is always enabled (https://github.com/emscripten-core/emscripten/pull/7935)'],
  ['MEMFS_APPEND_TO_TYPED_ARRAYS', [1], 'Starting from Emscripten 1.38.26, MEMFS_APPEND_TO_TYPED_ARRAYS=0 is no longer supported. MEMFS no longer supports using JS arrays for file data (https://github.com/emscripten-core/emscripten/pull/7918)'],
  ['ERROR_ON_MISSING_LIBRARIES', [1], 'missing libraries are always an error now'],
  ['EMITTING_JS', [1], 'The new STANDALONE_WASM flag replaces this (replace EMITTING_JS=0 with STANDALONE_WASM=1)'],
  ['SKIP_STACK_IN_SMALL', [0, 1], 'SKIP_STACK_IN_SMALL is no longer needed as the backend can optimize it directly'],
  ['SAFE_STACK', [0], 'Replace SAFE_STACK=1 with STACK_OVERFLOW_CHECK=2'],
  ['MEMORY_GROWTH_STEP', 'MEMORY_GROWTH_LINEAR_STEP'],
  ['ELIMINATE_DUPLICATE_FUNCTIONS', [0, 1], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
  ['ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS', [0], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
  ['ELIMINATE_DUPLICATE_FUNCTIONS_PASSES', [5], 'Duplicate function elimination for wasm is handled automatically by binaryen'],
  // WASM_OBJECT_FILES is handled in emcc.py, supporting both 0 and 1 for now.
  ['WASM_OBJECT_FILES', [0, 1], 'For LTO, use -flto or -fto=thin instead; to disable LTO, just do not pass WASM_OBJECT_FILES=1 as 1 is the default anyhow'],
  ['TOTAL_MEMORY', 'INITIAL_MEMORY'],
  ['WASM_MEM_MAX', 'MAXIMUM_MEMORY'],
  ['BINARYEN_MEM_MAX', 'MAXIMUM_MEMORY'],
  ['BINARYEN_PASSES', [''], 'Use BINARYEN_EXTRA_PASSES to add additional passes'],
  ['SWAPPABLE_ASM_MODULE', [0], 'Fully swappable asm modules are no longer supported'],
  ['ASM_JS', [1], 'asm.js output is not supported any more'],
  ['FINALIZE_ASM_JS', [0, 1], 'asm.js output is not supported any more'],
  ['ASYNCIFY_WHITELIST', 'ASYNCIFY_ONLY'],
  ['ASYNCIFY_BLACKLIST', 'ASYNCIFY_REMOVE'],
  ['EXCEPTION_CATCHING_WHITELIST', 'EXCEPTION_CATCHING_ALLOWED'],
  ['SEPARATE_ASM', [0], 'Separate asm.js only made sense for fastcomp with asm.js output'],
  ['SEPARATE_ASM_MODULE_NAME', [''], 'Separate asm.js only made sense for fastcomp with asm.js output'],
  ['FAST_UNROLLED_MEMCPY_AND_MEMSET', [0, 1], 'The wasm backend implements memcpy/memset in C'],
  ['DOUBLE_MODE', [0, 1], 'The wasm backend always implements doubles normally'],
  ['PRECISE_F32', [0, 1, 2], 'The wasm backend always implements floats normally'],
  ['ALIASING_FUNCTION_POINTERS', [0, 1], 'The wasm backend always uses a single index space for function pointers, in a single Table'],
  ['AGGRESSIVE_VARIABLE_ELIMINATION', [0, 1], 'Wasm ignores asm.js-specific optimization flags'],
  ['SIMPLIFY_IFS', [1], 'Wasm ignores asm.js-specific optimization flags'],
  ['DEAD_FUNCTIONS', [[]], 'The wasm backend does not support dead function removal'],
  ['WASM_BACKEND', [-1], 'Only the wasm backend is now supported (note that setting it as -s has never been allowed anyhow)'],
  ['EXPORT_BINDINGS', [0, 1], 'No longer needed'],
  ['RUNNING_JS_OPTS', [0], 'Fastcomp cared about running JS which could alter asm.js validation, but not upstream'],
  ['EXPORT_FUNCTION_TABLES', [0], 'No longer needed'],
  ['BINARYEN_SCRIPTS', [""], 'No longer needed'],
  ['WARN_UNALIGNED', [0, 1], 'No longer needed'],
  ['ASM_PRIMITIVE_VARS', [[]], 'No longer needed'],
  ['WORKAROUND_IOS_9_RIGHT_SHIFT_BUG', [0], 'Wasm2JS does not support iPhone 4s, iPad 2, iPad 3, iPad Mini 1, Pod Touch 5 (devices with end-of-life at iOS 9.3.5) and older']
];
