
//
// Various compiling-to-JS parameters. These are simply variables present when the
// JS compiler runs. To set them, do something like
//
//   emcc -s OPTION1=VALUE1 -s OPTION2=VALUE2 [..other stuff..]
//
// See https://github.com/kripken/emscripten/wiki/Code-Generation-Modes/
//
// Note that the values here are the defaults in -O0, that is, unoptimized
// mode. See apply_opt_level in tools/shared.py for how -O1,2,3 affect these
// flags.
//
// These flags should only have an effect when compiling to JS, so there
// should not be a need to have them when just compiling source to
// bitcode. However, there will also be no harm either, so it is ok to.
//

// Tuning
var QUANTUM_SIZE = 4; // This is the size of an individual field in a structure. 1 would
                      // lead to e.g. doubles and chars both taking 1 memory address. This
                      // is a form of 'compressed' memory, with shrinking and stretching
                      // according to the type, when compared to C/C++. On the other hand
                      // the normal value of 4 means all fields take 4 memory addresses,
                      // as per the norm on a 32-bit machine.
                      //
                      // Changing this from the default of 4 is deprecated.

var ASSERTIONS = 1; // Whether we should add runtime assertions, for example to
                    // check that each allocation to the stack does not
                    // exceed its size, whether all allocations (stack and static) are
                    // of positive size, etc., whether we should throw if we encounter a bad __label__, i.e.,
                    // if code flow runs into a fault
                    // ASSERTIONS == 2 gives even more runtime checks
var STACK_OVERFLOW_CHECK = 0; // Chooses what kind of stack smash checks to emit to generated code:
                              // 0: Stack overflows are not checked.
                              // 1: Adds a security cookie at the top of the stack, which is checked at end of each tick and at exit (practically zero performance overhead)
                              // 2: Same as above, but also adds an explicit check for allocate() calls which call ALLOC_STACK. Has a small performance cost.
                              //    -s ASSERTIONS=1 automatically enables -s STACK_OVERFLOW_CHECK=2.
var VERBOSE = 0; // When set to 1, will generate more verbose output during compilation.

var INVOKE_RUN = 1; // Whether we will run the main() function. Disable if you embed the generated
                    // code in your own, and will call main() yourself at the right time (which you
                    // can do with Module.callMain(), with an optional parameter of commandline args).
var NO_EXIT_RUNTIME = 0; // If set, the runtime is not quit when main() completes (allowing code to
                         // run afterwards, for example from the browser main event loop).
var MEM_INIT_METHOD = 0; // How to represent the initial memory content.
                         // 0: keep array literal representing the initial memory data
                         // 1: create a *.mem file containing the binary data of the initial memory;
                         //    use the --memory-init-file command line switch to select this method
                         // 2: embed a string literal representing that initial memory data
                         //    XXX this is known to have bugs on windows, see https://github.com/kripken/emscripten/pull/3326
var TOTAL_STACK = 5*1024*1024; // The total stack size. There is no way to enlarge the stack, so this
                               // value must be large enough for the program's requirements. If
                               // assertions are on, we will assert on not exceeding this, otherwise,
                               // it will fail silently.
var TOTAL_MEMORY = 16777216;     // The total amount of memory to use. Using more memory than this will
                                 // cause us to expand the heap, which can be costly with typed arrays:
                                 // we need to copy the old heap into a new one in that case.
var ABORTING_MALLOC = 1; // If 1, then when malloc would fail we abort(). This is nonstandard behavior,
                         // but makes sense for the web since we have a fixed amount of memory that
                         // must all be allocated up front, and so (a) failing mallocs are much more
                         // likely than on other platforms, and (b) people need a way to find out
                         // how big that initial allocation (TOTAL_MEMORY) must be.
                         // If you set this to 0, then you get the standard malloc behavior of
                         // returning NULL (0) when it fails.
var ALLOW_MEMORY_GROWTH = 0; // If false, we abort with an error if we try to allocate more memory than
                             // we can (TOTAL_MEMORY). If true, we will grow the memory arrays at
                             // runtime, seamlessly and dynamically. This has a performance cost though,
                             // both during the actual growth and in general (the latter is because in
                             // that case we must be careful about optimizations, in particular the
                             // eliminator).
                             // See https://code.google.com/p/v8/issues/detail?id=3907 regarding
                             // memory growth performance in chrome.
                             // Setting this option on will disable ABORTING_MALLOC, in other words,
                             // ALLOW_MEMORY_GROWTH enables fully standard behavior, of both malloc
                             // returning 0 when it fails, and also of being able to allocate more
                             // memory from the system as necessary.

var GLOBAL_BASE = -1; // where global data begins; the start of static memory. -1 means use the
                      // default, any other value will be used as an override

// Code embetterments
var DOUBLE_MODE = 1; // How to load and store 64-bit doubles.
                     // A potential risk is that doubles may be only 32-bit aligned. Forcing 64-bit alignment
                     // in Clang itself should be able to solve that, or as a workaround in DOUBLE_MODE 1 we
                     // will carefully load in parts, in a way that requires only 32-bit alignment. In DOUBLE_MODE
                     // 0 we will simply store and load doubles as 32-bit floats, so when they are stored/loaded
                     // they will truncate from 64 to 32 bits, and lose precision. This is faster, and might
                     // work for some code (but probably that code should just use floats and not doubles anyhow).
                     // Note that a downside of DOUBLE_MODE 1 is that we currently store the double in parts,
                     // then load it aligned, and that load-store will make JS engines alter it if it is being
                     // stored to a typed array for security reasons. That will 'fix' the number from being a
                     // NaN or an infinite number.
var UNALIGNED_MEMORY = 0; // If enabled, all memory accesses are assumed to be unaligned.  In unaligned memory mode,
                          // you can run nonportable code that typically would break in JS (or on ARM for that
                          // matter, which also cannot do unaligned reads/writes), at the cost of slowness
var FORCE_ALIGNED_MEMORY = 0; // If enabled, assumes all reads and writes are fully aligned for the type they
                              // use. This is true in proper C code (no undefined behavior), but is sadly
                              // common enough that we can't do it by default. See SAFE_HEAP.
                              // for ways to help find places in your code where unaligned reads/writes are done -
                              // you might be able to refactor your codebase to prevent them, which leads to
                              // smaller and faster code, or even the option to turn this flag on.
var WARN_UNALIGNED = 0; // Warn at compile time about instructions that LLVM tells us are not fully aligned.
                        // This is useful to find places in your code where you might refactor to ensure proper
                        // alignment.
var PRECISE_I64_MATH = 1; // If enabled, i64 addition etc. is emulated - which is slow but precise. If disabled,
                          // we use the 'double trick' which is fast but incurs rounding at high values.
                          // If set to 2, we always include the i64 math code, which is necessary in the case
                          // that we can't know at compile time that 64-bit math is needed. For example, if you
                          // print 64-bit values with printf, but never add them, we can't know at compile time
                          // and you need to set this to 2.
var PRECISE_F32 = 0; // 0: Use JS numbers for floating-point values. These are 64-bit and do not model C++
                     //    floats exactly, which are 32-bit.
                     // 1: Model C++ floats precisely, using Math.fround, polyfilling when necessary. This
                     //    can be slow if the polyfill is used on heavy float32 computation. See note on
                     //    browser support below.
                     // 2: Model C++ floats precisely using Math.fround if available in the JS engine, otherwise
                     //    use an empty polyfill. This will have much less of a speed penalty than using the full
                     //    polyfill in cases where engine support is not present. In addition, we can
                     //    remove the empty polyfill calls themselves on the client when generating html,
                     //    which should mean that this gives you the best of both worlds of 0 and 1, and is
                     //    therefore recommended, *unless* you need a guarantee of proper float32 precision
                     //    (in that case, use option 1).
                     // XXX Note: To optimize float32-using code, we use the 'const' keyword in the emitted
                     //           code. This allows us to avoid unnecessary calls to Math.fround, which would
                     //           slow down engines not yet supporting that function. 'const' is present in
                     //           all modern browsers, including Firefox, Chrome and Safari, but in IE is only
                     //           present in IE11 and above. Therefore if you need to support legacy versions of
                     //           IE, you should not enable PRECISE_F32 1 or 2.
var SIMD = 0; // Whether to allow autovectorized SIMD code ( https://github.com/johnmccutchan/ecmascript_simd ).
              // SIMD intrinsics are always compiled to SIMD code, so you only need this option if you
              // also want the autovectorizer to run.
              // Note that SIMD support in browsers is not yet there (as of Sep 2, 2014), so you will be
              // running in a polyfill, which is not fast.

var USE_CLOSURE_COMPILER = 0; // Whether closure compiling is being run on this output

var SKIP_STACK_IN_SMALL = 1; // When enabled, does not push/pop the stack at all in
                             // functions that have no basic stack usage. But, they
                             // may allocate stack later, and in a loop, this can be
                             // very bad. In particular, when debugging, printf()ing
                             // a lot can exhaust the stack very fast, with this option.
                             // In particular, be careful with the autodebugger! (We do turn
                             // this off automatically in that case, though.)
var INLINING_LIMIT = 0;  // A limit on inlining. If 0, we will inline normally in LLVM and
                         // closure. If greater than 0, we will *not* inline in LLVM, and
                         // we will prevent inlining of functions of this size or larger
                         // in closure. 50 is a reasonable setting if you do not want
                         // inlining
var OUTLINING_LIMIT = 0; // A function size above which we try to automatically break up
                         // functions into smaller ones, to avoid the downsides of very
                         // large functions (JS engines often compile them very slowly,
                         // compile them with lower optimizations, or do not optimize them
                         // at all). If 0, we do not perform outlining at all.
                         // To see which funcs are large, you can inspect the source
                         // in a debug build (-g2 or -g for example), and can run
                         // tools/find_bigfuncs.py on that to get a sorted list by size.
                         // Another possibility is to look in the web console in firefox,
                         // which will note slowly-compiling functions.
                         // You will probably want to experiment with various values to
                         // see the impact on compilation time, code size and runtime
                         // throughput. It is hard to say what values to start testing
                         // with, but something around 20,000 to 100,000 might make sense.
                         // (The unit size is number of AST nodes.)
                         // Outlining decreases maximum function size, but does so at the
                         // cost of increasing overall code size as well as performance
                         // (outlining itself makes code less optimized, and requires
                         // emscripten to disable some passes that are incompatible with
                         // it).

var AGGRESSIVE_VARIABLE_ELIMINATION = 0; // Run aggressiveVariableElimination in js-optimizer.js
var SIMPLIFY_IFS = 1; // Whether to simplify ifs in js-optimizer.js

// Generated code debugging options
var SAFE_HEAP = 0; // Check each write to the heap, for example, this will give a clear
                   // error on what would be segfaults in a native build (like dereferencing
                   // 0). See preamble.js for the actual checks performed.
var SAFE_HEAP_LOG = 0; // Log out all SAFE_HEAP operations

var RESERVED_FUNCTION_POINTERS = 0; // In asm.js mode, we cannot simply add function pointers to
                                    // function tables, so we reserve some slots for them. An
                                    // alternative to this is to use EMULATED_FUNCTION_POINTERS,
                                    // in which case we don't need to reserve.
var ALIASING_FUNCTION_POINTERS = 0; // Whether to allow function pointers to alias if they have
                                    // a different type. This can greatly decrease table sizes
                                    // in asm.js, but can break code that compares function
                                    // pointers across different types.
var EMULATED_FUNCTION_POINTERS = 0; // By default we implement function pointers using asm.js
                                    // function tables, which is very fast. With this option,
                                    // we implement them more flexibly by emulating them: we
                                    // call out into JS, which handles the function tables.
                                    //  1: Full emulation. This means you can modify the
                                    //     table in JS fully dynamically, not just add to
                                    //     the end.
                                    //  2: Optimized emulation. Assumes once something is
                                    //     added to the table, it will not change. This allows
                                    //     dynamic linking while keeping performance fast,
                                    //     as we can do a fast call into the internal table
                                    //     if the fp is in the right range. Shared modules
                                    //     (MAIN_MODULE, SIDE_MODULE) do this by default.
                                    //     This requires RELOCATABLE to be set.
var EMULATE_FUNCTION_POINTER_CASTS = 0; // Allows function pointers to be cast, wraps each
                                        // call of an incorrect type with a runtime correction.
                                        // This adds overhead and should not be used normally.
                                        // It also forces ALIASING_FUNCTION_POINTERS to 0.
var FUNCTION_POINTER_ALIGNMENT = 2; // Byte alignment of function pointers - we will fill the
                                    // tables with zeros on aligned values. 1 means all values
                                    // are aligned and all will be used (which is optimal).
                                    // Sadly 1 breaks on &Class::method function pointer calls,
                                    // which llvm assumes have the lower bit zero (see
                                    // test_polymorph and issue #1692).

var EXCEPTION_DEBUG = 0; // Print out exceptions in emscriptened code. Does not work in asm.js mode

var DEMANGLE_SUPPORT = 0; // If 1, build in libcxxabi's full c++ demangling code, to allow stackTrace()
                          // to emit fully proper demangled c++ names

var LIBRARY_DEBUG = 0; // Print out when we enter a library call (library*.js). You can also unset
                       // Runtime.debug at runtime for logging to cease, and can set it when you
                       // want it back. A simple way to set it in C++ is
                       //   emscripten_run_script("Runtime.debug = ...;");
var SYSCALL_DEBUG = 0; // Print out all syscalls
var SOCKET_DEBUG = 0; // Log out socket/network data transfer.
var SOCKET_WEBRTC = 0; // Select socket backend, either webrtc or websockets. XXX webrtc is not currently tested, may be broken

// As well as being configurable at compile time via the "-s" option the WEBSOCKET_URL and WEBSOCKET_SUBPROTOCOL
// settings may configured at run time via the Module object e.g.
// Module['websocket'] = {subprotocol: 'base64, binary, text'};
// Module['websocket'] = {url: 'wss://', subprotocol: 'base64'};
// Run time configuration may be useful as it lets an application select multiple different services.
var WEBSOCKET_URL = 'ws://'; // A string containing either a WebSocket URL prefix (ws:// or wss://) or a complete
                             // RFC 6455 URL - "ws[s]:" "//" host [ ":" port ] path [ "?" query ].
                             // In the (default) case of only a prefix being specified the URL will be constructed from
                             // prefix + addr + ':' + port
                             // where addr and port are derived from the socket connect/bind/accept calls.
var WEBSOCKET_SUBPROTOCOL = 'binary'; // A string containing a comma separated list of WebSocket subprotocols
                                      // as would be present in the Sec-WebSocket-Protocol header.

var OPENAL_DEBUG = 0; // Print out debugging information from our OpenAL implementation.

var GL_ASSERTIONS = 0; // Adds extra checks for error situations in the GL library. Can impact performance.
var GL_DEBUG = 0; // Print out all calls into WebGL. As with LIBRARY_DEBUG, you can set a runtime
                  // option, in this case GL.debug.
var GL_TESTING = 0; // When enabled, sets preserveDrawingBuffer in the context, to allow tests to work (but adds overhead)
var GL_MAX_TEMP_BUFFER_SIZE = 2097152; // How large GL emulation temp buffers are
var GL_UNSAFE_OPTS = 1; // Enables some potentially-unsafe optimizations in GL emulation code
var FULL_ES2 = 0;   // Forces support for all GLES2 features, not just the WebGL-friendly subset.
var USE_WEBGL2 = 0; // Enables WebGL2 native functions. This mode will also create a WebGL2
                    // context by default if no version is specified.
var FULL_ES3 = 0;   // Forces support for all GLES3 features, not just the WebGL2-friendly subset.
var LEGACY_GL_EMULATION = 0; // Includes code to emulate various desktop GL features. Incomplete but useful
                             // in some cases, see http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
var GL_FFP_ONLY = 0; // If you specified LEGACY_GL_EMULATION = 1 and only use fixed function pipeline in your code,
                     // you can also set this to 1 to signal the GL emulation layer that it can perform extra
                     // optimizations by knowing that the user code does not use shaders at all. If
                     // LEGACY_GL_EMULATION = 0, this setting has no effect.

var STB_IMAGE = 0; // Enables building of stb-image, a tiny public-domain library for decoding images, allowing
                   // decoding of images without using the browser's built-in decoders. The benefit is that this
                   // can be done synchronously, however, it will not be as fast as the browser itself.
                   // When enabled, stb-image will be used automatically from IMG_Load and IMG_Load_RW. You
                   // can also call the stbi_* functions directly yourself.

var LZ4 = 0; // Enable this to support lz4-compressed file packages. They are stored compressed in memory, and
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

var DISABLE_EXCEPTION_CATCHING = 0; // Disables generating code to actually catch exceptions. If the code you
                                    // are compiling does not actually rely on catching exceptions (but the
                                    // compiler generates code for it, maybe because of stdlibc++ stuff),
                                    // then this can make it much faster. If an exception actually happens,
                                    // it will not be caught and the program will halt (so this will not
                                    // introduce silent failures, which is good).
                                    // DISABLE_EXCEPTION_CATCHING = 0 - generate code to actually catch exceptions
                                    // DISABLE_EXCEPTION_CATCHING = 1 - disable exception catching at all
                                    // DISABLE_EXCEPTION_CATCHING = 2 - disable exception catching, but enables
                                    // catching in whitelist
                                    // TODO: Make this also remove cxa_begin_catch etc., optimize relooper
                                    //       for it, etc. (perhaps do all of this as preprocessing on .ll?)

var EXCEPTION_CATCHING_WHITELIST = [];  // Enables catching exception in the listed functions only, if
                                        // DISABLE_EXCEPTION_CATCHING = 2 is set

// For more explanations of this option, please visit
// https://github.com/kripken/emscripten/wiki/Asyncify
var ASYNCIFY = 0; // Whether to enable asyncify transformation
                  // This allows to inject some async functions to the C code that appear to be sync
                  // e.g. emscripten_sleep
var ASYNCIFY_FUNCTIONS = ['emscripten_sleep', // Functions that call any function in the list, directly or indirectly
                          'emscripten_wget',  // will be transformed
                          'emscripten_yield'];
var ASYNCIFY_WHITELIST = ['qsort',   // Functions in this list are never considered async, even if they appear in ASYNCIFY_FUNCTIONS
                          'trinkle', // In the asyncify transformation, any function that calls a function pointer is considered async
                          '__toread', // This whitelist is useful when a function is known to be sync
                          '__uflow',  // currently this link contains some functions in libc
                          '__fwritex',
                          'MUSL_vfprintf'];

var EXPORTED_RUNTIME_METHODS = [ // Runtime elements that are exported on Module. By default we export quite a bit, you can reduce this list to lower your code size,
                                 // especially when closure is run (exporting prevents closure from eliminating code)
                                 // Note that methods on this list are only exported if they are included (either automatically from linking, or due to being
                                 // in DEFAULT_LIBRARY_FUNCS_TO_INCLUDE)
                                 // Note that the name may be slightly misleading, as this
                                 // is for any JS library element, and not just
                                 // methods. For example, we export the Runtime object
                                 // by having "Runtime" in this list.
  'FS_createFolder',
  'FS_createPath',
  'FS_createDataFile',
  'FS_createPreloadedFile',
  'FS_createLazyFile',
  'FS_createLink',
  'FS_createDevice',
  'FS_unlink',
  'Runtime',
  'ccall',
  'cwrap',
  'setValue',
  'getValue',
  'ALLOC_NORMAL',
  'ALLOC_STACK',
  'ALLOC_STATIC',
  'ALLOC_DYNAMIC',
  'ALLOC_NONE',
  'allocate',
  'getMemory',
  'Pointer_stringify',
  'AsciiToString',
  'stringToAscii',
  'UTF8ArrayToString',
  'UTF8ToString',
  'stringToUTF8Array',
  'stringToUTF8',
  'lengthBytesUTF8',
  'stackTrace',
  'addOnPreRun',
  'addOnInit',
  'addOnPreMain',
  'addOnExit',
  'addOnPostRun',
  'intArrayFromString',
  'intArrayToString',
  'writeStringToMemory',
  'writeArrayToMemory',
  'writeAsciiToMemory',
  'addRunDependency',
  'removeRunDependency',
];

var EXTRA_EXPORTED_RUNTIME_METHODS = []; // Additional methods to those in EXPORTED_RUNTIME_METHODS. Adjusting that list
                                         // lets you remove methods that would be exported by default; setting values in
                                         // this list lets you add to the default list without modifying it.

var FS_LOG = 0; // Log all FS operations.  This is especially helpful when you're porting
                // a new project and want to see a list of file system operations happening
                // so that you can create a virtual file system with all of the required files.
var CASE_INSENSITIVE_FS = 0; // If set to nonzero, the provided virtual filesystem if treated case-insensitive, like
                             // Windows and OSX do. If set to 0, the VFS is case-sensitive, like on Linux.
var MEMFS_APPEND_TO_TYPED_ARRAYS = 0; // If set to nonzero, MEMFS will always utilize typed arrays as the backing store
                                      // for appending data to files. The default behavior is to use typed arrays for files
                                      // when the file size doesn't change after initial creation, and for files that do
                                      // change size, use normal JS arrays instead.
var NO_FILESYSTEM = 0; // If set, does not build in any filesystem support. Useful if you are just doing pure
                       // computation, but not reading files or using any streams (including fprintf, and other
                       // stdio.h things) or anything related. The one exception is there is partial support for printf,
                       // and puts, hackishly.
                       // The compiler will automatically set this if it detects that syscall usage (which is static)
                       // does not require a full filesystem. If you still want filesystem support, use
                       // FORCE_FILESYSTEM
var FORCE_FILESYSTEM = 0; // Makes full filesystem support be included, even if statically it looks like it is not
                          // used. For example, if your C code uses no files, but you include some JS that does,
                          // you might need this.

var EXPORTED_FUNCTIONS = ['_main'];
                                    // Functions that are explicitly exported. These functions are kept alive
                                    // through LLVM dead code elimination, and also made accessible outside of
                                    // the generated code even after running closure compiler (on "Module").
                                    // Note the necessary prefix of "_".
                                    // Note also that this is the full list of exported functions - if you
                                    // have a main() function and want it to run, you must include it in this
                                    // list (as _main is by default in this value, and if you override it
                                    // without keeping it there, you are in effect removing it).

var EXPORT_ALL = 0; // If true, we export all the symbols. Note that this does *not* affect LLVM, so it can
                    // still eliminate functions as dead. This just exports them on the Module object.
var EXPORT_BINDINGS = 0; // Export all bindings generator functions (prefixed with emscripten_bind_). This
                         // is necessary to use the WebIDL binder or bindings generator with asm.js
var EXPORT_FUNCTION_TABLES = 0; // If true, export all the functions appearing in a function table, and the
                                // tables themselves.
var RETAIN_COMPILER_SETTINGS = 0; // Remembers the values of these settings, and makes them accessible
                                  // through Runtime.getCompilerSetting and emscripten_get_compiler_setting.
                                  // To see what is retained, look for compilerSettings in the generated code.


var EMSCRIPTEN_VERSION = ''; // this will contain the emscripten version. you should not modify it. This
                             // and the following few settings are useful in combination with
                             // RETAIN_COMPILER_SETTINGS
var OPT_LEVEL = 0;           // this will contain the optimization level (-Ox). you should not modify it.
var DEBUG_LEVEL = 0;         // this will contain the debug level (-gx). you should not modify it.


// JS library elements (C functions implemented in JS)
// that we include by default. If you want to make sure
// something is included by the JS compiler, add it here.
// For example, if you do not use some emscripten_*
// C API call from C, but you want to call it from JS,
// add it here (and in EXPORTED FUNCTIONS with prefix
// "_", if you use closure compiler).
// Note that the name may be slightly misleading, as this
// is for any JS library element, and not just
// functions. For example, you can include the Browser
// object by adding "$Browser" to this list.
var DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = ['memcpy', 'memset', 'malloc', 'free'];

var LIBRARY_DEPS_TO_AUTOEXPORT = ['memcpy']; // This list is also used to determine
                                             // auto-exporting of library dependencies (i.e., functions that
                                             // might be dependencies of JS library functions, that if
                                             // so we must export so that if they are implemented in C
                                             // they will be accessible, in ASM_JS mode).

var INCLUDE_FULL_LIBRARY = 0; // Include all JS library functions instead of the sum of
                              // DEFAULT_LIBRARY_FUNCS_TO_INCLUDE + any functions used
                              // by the generated code. This is needed when dynamically
                              // loading (i.e. dlopen) modules that make use of runtime
                              // library functions that are not used in the main module.
                              // Note that this only applies to js libraries, *not* C. You
                              // will need the main file to include all needed C libraries.
                              // For example, if a module uses malloc or new, you will
                              // need to use those in the main file too to pull in dlmalloc
                              // for use by the module.

var SHELL_FILE = 0; // set this to a string to override the shell file used

var RELOCATABLE = 0; // If set to 1, we emit relocatable code from the LLVM backend; both
                     // globals and function pointers are all offset (by gb and fp, respectively)

var MAIN_MODULE = 0; // A main module is a file compiled in a way that allows us to link it to
                     // a side module using emlink.py.
                     //  1: Normal main module.
                     //  2: DCE'd main module. We eliminate dead code normally. If a side
                     //     module needs something from main, it is up to you to make sure
                     //     it is kept alive.
var SIDE_MODULE = 0; // Corresponds to MAIN_MODULE

var RUNTIME_LINKED_LIBS = []; // If this is a main module (MAIN_MODULE == 1), then
                              // we will link these at runtime. They must have been built with
                              // SIDE_MODULE == 1.
var BUILD_AS_SHARED_LIB = 0; // (deprecated option TODO: remove)

var BUILD_AS_WORKER = 0; // If set to 1, this is a worker library, a special kind of library
                         // that is run in a worker. See emscripten.h

var PROXY_TO_WORKER = 0; // If set to 1, we build the project into a js file that will run
                         // in a worker, and generate an html file that proxies input and
                         // output to/from it.
var PROXY_TO_WORKER_FILENAME = ''; // If set, the script file name the main thread loads.
                                   // Useful if your project doesn't run the main emscripten-
                                   // generated script immediately but does some setup before

var LINKABLE = 0; // If set to 1, this file can be linked with others, either as a shared
                  // library or as the main file that calls a shared library. To enable that,
                  // we will not internalize all symbols and cull the unused ones, in other
                  // words, we will not remove unused functions and globals, which might be
                  // used by another module we are linked with.
                  // BUILD_AS_SHARED_LIB > 0 implies this, so it is only important to set this to 1
                  // when building the main file, and *if* that main file has symbols that
                  // the library it will open will then access through an extern.
                  // LINKABLE of 0 is very useful in that we can reduce the size of the
                  // generated code very significantly, by removing everything not actually used.

var WARN_ON_UNDEFINED_SYMBOLS = 1; // If set to 1, we will warn on any undefined symbols that
                                   // are not resolved by the library_*.js files. Note that
                                   // it is common in large projects to
                                   // not implement everything, when you know what is not
                                   // going to actually be called (and don't want to mess with
                                   // the existing buildsystem), and functions might be
                                   // implemented later on, say in --pre-js, so you may
                                   // want to build with -s WARN_ON_UNDEFINED_SYMBOLS=0 to
                                   // disable the warnings if they annoy you.
                                   // See also ERROR_ON_UNDEFINED_SYMBOLS

var ERROR_ON_UNDEFINED_SYMBOLS = 0; // If set to 1, we will give a compile-time error on any
                                    // undefined symbols (see WARN_ON_UNDEFINED_SYMBOLS).

var SMALL_XHR_CHUNKS = 0; // Use small chunk size for binary synchronous XHR's in Web Workers.
                          // Used for testing.
                          // See test_chunked_synchronous_xhr in runner.py and library.js.

var HEADLESS = 0; // If 1, will include shim code that tries to 'fake' a browser
                  // environment, in order to let you run a browser program (say,
                  // using SDL) in the shell. Obviously nothing is rendered, but
                  // this can be useful for benchmarking and debugging if actual
                  // rendering is not the issue. Note that the shim code is
                  // very partial - it is hard to fake a whole browser! - so
                  // keep your expectations low for this to work.

var DETERMINISTIC = 0; // If 1, we force Date.now(), Math.random, etc. to return deterministic
                       // results. Good for comparing builds for debugging purposes (and nothing else)

var MODULARIZE = 0; // By default we emit all code in a straightforward way into the output
                    // .js file. That means that if you load that in a script tag in a web
                    // page, it will use the global scope. With MODULARIZE set, we will instead emit
                    //
                    //   var EXPORT_NAME = function(Module) {
                    //     Module = Module || {};
                    //     // .. all the emitted code from emscripten ..
                    //     return Module;
                    //   };
                    //
                    // where EXPORT_NAME is from the option of the same name (so, by default
                    // it will be var Module = ..., and so you should change EXPORT_NAME if
                    // you want more than one module in the same web page).
                    //
                    // You can then use this by something like
                    //
                    //   var instance = EXPORT_NAME();
                    //
                    // or
                    //
                    //   var instance = EXPORT_NAME({ option: value, ... });
                    //
                    // Note the parentheses - we are calling EXPORT_NAME in order to instantiate
                    // the module. (This allows, in particular, for you to create multiple
                    // instantiations, etc.)

var BENCHMARK = 0; // If 1, will just time how long main() takes to execute, and not
                   // print out anything at all whatsoever. This is useful for benchmarking.

var ASM_JS = 1; // If 1, generate code in asm.js format. If 2, emits the same code except
                // for omitting 'use asm'
var FINALIZE_ASM_JS = 1; // If 1, will finalize the final emitted code, including operations
                         // that prevent later js optimizer passes from running, like
                         // converting +5 into 5.0 (the js optimizer sees 5.0 as just 5).

var SWAPPABLE_ASM_MODULE = 0; // If 1, then all exports from the asm.js module will be accessed
                              // indirectly, which allow the asm module to be swapped later.
                              // Note: It is very important to build the two modules that
                              // are to be swapped with the same optimizations and so forth,
                              // as we depend on them being a drop-in replacement for each
                              // other (same globals on the heap at the same locations, etc.)

var SEPARATE_ASM = 0; // see emcc --separate-asm

var ONLY_MY_CODE = 0; // This disables linking and other causes of adding extra code
                      // automatically, and as a result, your output compiled code
                      // (in the .asm.js file, if you emit with --separate-asm) will
                      //  contain only the functions you provide.

var PGO = 0; // Enables profile-guided optimization in the form of runtime checks for
             // which functions are actually called. Emits a list during shutdown that you
             // can pass to DEAD_FUNCTIONS (you can also emit the list manually by
             // calling PGOMonitor.dump());
var DEAD_FUNCTIONS = []; // JS library functions on this list are not converted to JS, and calls to
                         // them are turned into abort()s. This is potentially useful for
                         // reducing code size.
                         // If a dead function is actually called, you will get a runtime
                         // error.
                         // TODO: make this work on compiled methods as well, perhaps by
                         //       adding a JS optimizer pass?

var EXPLICIT_ZEXT = 0; // If 1, generate an explicit conversion of zext i1 to i32, using ?:

var EXPORT_NAME = 'Module'; // Global variable to export the module as for environments without a standardized module
                            // loading system (e.g. the browser and SM shell).

var NO_DYNAMIC_EXECUTION = 0; // When set to 1, we do not emit eval() and new Function(), which disables some functionality
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
                              // NO_DYNAMIC_EXECUTION=1 is set, and an attempt to call them will throw an exception:
                              // - emscripten_run_script(),
                              // - emscripten_run_script_int(),
                              // - emscripten_run_script_string(),
                              // - dlopen(),
                              // - the functions ccall() and cwrap() are still available, but they are restricted to only
                              //   being able to call functions that have been exported in the Module object in advance.
                              // When set to -s NO_DYNAMIC_EXECUTION=2 flag is set, attempts to call to eval() are demoted
                              // to warnings instead of throwing an exception.

var EMTERPRETIFY = 0; // Runs tools/emterpretify on the compiler output
var EMTERPRETIFY_FILE = ''; // If defined, a file to write bytecode to, otherwise the default is to embed it in text JS arrays (which is less efficient).
                            // When emitting HTML, we automatically generate code to load this file and set it to Module.emterpreterFile. If you
                            // emit JS, you need to make sure that Module.emterpreterFile contains an ArrayBuffer with the bytecode, when the code loads.
                            // Note: You might need to quote twice in the shell, something like     -s 'EMTERPRETIFY_FILE="waka"'
var EMTERPRETIFY_BLACKLIST = []; // Functions to not emterpret, that is, to run normally at full speed
var EMTERPRETIFY_WHITELIST = []; // If this contains any functions, then only the functions in this list
                                 // are emterpreted (as if all the rest are blacklisted; this overrides the BLACKLIST)
var EMTERPRETIFY_ASYNC = 0; // Allows sync code in the emterpreter, by saving the call stack, doing an async delay, and resuming it
var EMTERPRETIFY_ADVISE = 0; // Performs a static analysis to suggest which functions should be run in the emterpreter, as it
                             // appears they can be on the stack when a sync function is called in the EMTERPRETIFY_ASYNC option.
                             // After showing the suggested list, compilation will halt. You can apply the provided list as an
                             // emcc argument when compiling later.

var SPLIT_MEMORY = 0; // If > 0, we split memory into chunks, of the size given in this parameter.
                      //  * TOTAL_MEMORY becomes the maximum amount of memory, as chunks are allocated on
                      //    demand. That means this achieves a result similar to ALLOW_MEMORY_GROWTH, but
                      //    better since it can free chunks in the middle. You still to set
                      //    ALLOW_MEMORY_GROWTH if you want memory to grow beyond the initial TOTAL_MEMORY
                      //    target.
                      //  * Larger SPLIT_MEMORY sizes are generally faster to run.
                      // TODO: more docs
                      // TODO: add malloc-split to embuilder
var SAFE_SPLIT_MEMORY = 0; // Similar to SAFE_HEAP, but for SPLIT_MEMORY.

var RUNNING_JS_OPTS = 0; // whether js opts will be run, after the main compiler
var BOOTSTRAPPING_STRUCT_INFO = 0; // whether we are in the generate struct_info bootstrap phase

var EMSCRIPTEN_TRACING = 0; // Add some calls to emscripten tracing APIs

var USE_GLFW = 2; // Specify the GLFW version that is being linked against.
                  // Only relevant, if you are linking against the GLFW library.
                  // Valid options are 2 for GLFW2 and 3 for GLFW3.

var BINARYEN = 0; // Whether to use [Binaryen](https://github.com/WebAssembly/binaryen) to
                  // compile (at runtime) our asm.js output into WebAssembly.
                  // This will fetch the binaryen port and build it. (If, instead, you set
                  // BINARYEN_ROOT in your ~/.emscripten file, then we use that instead
                  // of the port, which can useful for local dev work on binaryen itself).
var BINARYEN_METHOD = ""; // See binaryen's src/js/post.js for details.
var BINARYEN_SCRIPTS = ""; // An optional comma-separated list of script hooks to run after binaryen,
                           // in binaryen's /scripts dir.
var BINARYEN_IMPRECISE = 0; // Whether to apply imprecise/unsafe binaryen optimizations. If enabled,
                            // code will run faster, but some types of undefined behavior might
                            // trap in wasm.
var BINARYEN_ROOT = ""; // Directory where we can find Binaryen. Will be automatically set for you,
                        // but you can set it to override if you are a Binaryen developer.

var WASM_BACKEND = 0; // Whether to use the WebAssembly backend that is in development in LLVM.
                      // This requires that BINARYEN be set, as we use Binaryen's s2wasm to
                      // translate the backend output.
                      // You should not set this yourself, instead set EMCC_WASM_BACKEND=1 in the
                      // environment.

// Ports

var USE_SDL = 1; // Specify the SDL version that is being linked against.
                 // 1, the default, is 1.3, which is implemented in JS
                 // 2 is a port of the SDL C code on emscripten-ports
var USE_SDL_IMAGE = 1; // Specify the SDL_image version that is being linked against. Must match USE_SDL
var USE_SDL_TTF = 1; // Specify the SDL_ttf version that is being linked against. Must match USE_SDL
var USE_SDL_NET = 1; // Specify the SDL_net version that is being linked against. Must match USE_SDL
var USE_ZLIB = 0; // 1 = use zlib from emscripten-ports
var USE_LIBPNG = 0; // 1 = use libpng from emscripten-ports
var USE_BULLET = 0; // 1 = use bullet from emscripten-ports
var USE_VORBIS = 0; // 1 = use vorbis from emscripten-ports
var USE_OGG = 0; // 1 = use ogg from emscripten-ports
var USE_FREETYPE = 0; // 1 = use freetype from emscripten-ports

var SDL2_IMAGE_FORMATS = []; // Formats to support in SDL2_image. Valid values: bmp, gif, lbm, pcx, png, pnm, tga, xcf, xpm, xv


// Compiler debugging options
var DEBUG_TAGS_SHOWING = [];
  // Some useful items:
  //    framework
  //    frameworkLines
  //    gconst
  //    types
  //    vars
  //    unparsedFunctions
  //    metadata
  //    legalizer

// For internal use only
var ORIGINAL_EXPORTED_FUNCTIONS = [];

// The list of defines (C_DEFINES) was moved into struct_info.json in the same directory.
// That file is automatically parsed by tools/gen_struct_info.py.
// If you modify the headers, just clear your cache and emscripten libc should see
// the new values.

var IN_TEST_HARNESS = 0; // If true, the current build is performed for the Emscripten test harness.

var USE_PTHREADS = 0; // If true, enables support for pthreads.

var PTHREAD_POOL_SIZE = 0; // Specifies the number of web workers that are preallocated before runtime is initialized. If 0, workers are created on demand.

// Specifies the value returned by the function emscripten_num_logical_cores()
// if navigator.hardwareConcurrency is not supported. Pass in a negative number
// to show a popup dialog at startup so the user can configure this dynamically.
var PTHREAD_HINT_NUM_CORES = 4;

var PTHREADS_PROFILING = 0; // True when building with --threadprofiler

var MAX_GLOBAL_ALIGN = -1; // received from the backend

// Duplicate function elimination. This coalesces function bodies that are
// identical, which can happen e.g. if two methods have different C/C++
// or LLVM types, but end up identical at the asm.js level (all pointers
// are the same as int32_t in asm.js, for example).
// This option is quite slow to run, as it processes and hashes all methods
// in the codebase in multiple passes.
var ELIMINATE_DUPLICATE_FUNCTIONS = 0; // disabled by default
var ELIMINATE_DUPLICATE_FUNCTIONS_PASSES = 5;
var ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS = 0;

var EVAL_CTORS = 0; // This tries to evaluate global ctors at compile-time, applying their
                    // effects into the mem init file. This saves running code during
                    // startup, and also allows removing the global ctor functions and
                    // other code that only they used, so this is also good for reducing
                    // code size. However, this does make the compile step much slower.
                    //
                    // This basically runs the ctors during compile time, seeing if they
                    // execute safely in a sandbox. Any ffi access out of asm.js causes
                    // failure, as it could do something nondeterministic and/or
                    // alter some other state we don't see. If all the global ctor does
                    // is pure computation inside asm.js, it should be ok. Run with
                    // EMCC_DEBUG=1 in the env to see logging, and errors when it
                    // fails to eval (you'll see a message, or a stack trace; in the
                    // latter case, the functions on the stack should give you an idea
                    // of what ffi was called and why, and perhaps you can refactor
                    // your code to avoid it, e.g., remove mallocs, printfs in global ctors).
                    //
                    // This optimization can increase the size of the mem init file,
                    // because ctors can write to memory that would otherwise be
                    // in a zeroinit area. This may not be a significant increase after
                    // gzip, if there are mostly zeros in there, and in any case
                    // the mem init increase would be offset by a code size decrease.
                    // (Unless you have a small ctor that writes 'random' data to memory,
                    // which would reduce little code but add potentially lots of
                    // uncompressible data.)
                    //
                    // LLVM's GlobalOpt *almost* does this operation. It does in simple
                    // cases, where LLVM IR is not too complex for its logic to evaluate,
                    // but it isn't powerful enough for e.g. libc++ iostream ctors. It
                    // is just hard to do at the LLVM IR level - LLVM IR is complex and
                    // getting more complex, this would require GlobalOpt to have a full
                    // interpreter, plus a way to write back into LLVM IR global objects.
                    // At the asm.js level, however, everything has been lowered into a
                    // simple low level, and we also just need to write bytes into an
                    // array, so this is easy for us to do, but not for LLVM. A further
                    // issue for LLVM is that it doesn't know that we will not link in
                    // further code, so it only tries to optimize ctors with lowest
                    // priority. We do know that, and can optimize all the ctors.

var CYBERDWARF = 0; // see http://kripken.github.io/emscripten-site/docs/debugging/CyberDWARF.html

var BUNDLED_CD_DEBUG_FILE = ""; // Path to the CyberDWARF debug file passed to the compiler

// Reserved: variables containing POINTER_MASKING.
