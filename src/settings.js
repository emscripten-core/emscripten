
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

// Tuning
var QUANTUM_SIZE = 4; // This is the size of an individual field in a structure. 1 would
                      // lead to e.g. doubles and chars both taking 1 memory address. This
                      // is a form of 'compressed' memory, with shrinking and stretching
                      // according to the type, when compared to C/C++. On the other hand
                      // the normal value of 4 means all fields take 4 memory addresses,
                      // as per the norm on a 32-bit machine.
                      //
                      // Changing this from the default of 4 is deprecated.

var TARGET_X86 = 0;  // For i386-pc-linux-gnu
var TARGET_ASMJS_UNKNOWN_EMSCRIPTEN = 1; // For asmjs-unknown-emscripten. 1 is normal, 2 is for the fastcomp llvm
                     // backend using emscripten-customized abi simplification

var CORRECT_SIGNS = 1; // Whether we make sure to convert unsigned values to signed values.
                       // Decreases performance with additional runtime checks. Might not be
                       // needed in some kinds of code.
                       // If equal to 2, done on a line-by-line basis according to
                       // CORRECT_SIGNS_LINES, correcting only the specified lines.
                       // If equal to 3, correcting all *but* the specified lines
var CHECK_SIGNS = 0; // Runtime errors for signing issues that need correcting.
                     // It is recommended to use this in
                     // order to find if your code needs CORRECT_SIGNS. If you can get your
                     // code to run without CORRECT_SIGNS, it will run much faster

var ASSERTIONS = 1; // Whether we should add runtime assertions, for example to
                    // check that each allocation to the stack does not
                    // exceed its size, whether all allocations (stack and static) are
                    // of positive size, etc., whether we should throw if we encounter a bad __label__, i.e.,
                    // if code flow runs into a fault
                    // ASSERTIONS == 2 gives even more runtime checks
var VERBOSE = 0; // When set to 1, will generate more verbose output during compilation.

var INVOKE_RUN = 1; // Whether we will run the main() function. Disable if you embed the generated
                    // code in your own, and will call main() yourself at the right time (which you
                    // can do with Module.callMain(), with an optional parameter of commandline args).
var NO_EXIT_RUNTIME = 0; // If set, the runtime is not quit when main() completes (allowing code to
                         // run afterwards, for example from the browser main event loop).
var INIT_HEAP = 0; // Whether to initialize memory anywhere other than the stack to 0.
var TOTAL_STACK = 5*1024*1024; // The total stack size. There is no way to enlarge the stack, so this
                               // value must be large enough for the program's requirements. If
                               // assertions are on, we will assert on not exceeding this, otherwise,
                               // it will fail silently.
var TOTAL_MEMORY = 16777216;     // The total amount of memory to use. Using more memory than this will
                                 // cause us to expand the heap, which can be costly with typed arrays:
                                 // we need to copy the old heap into a new one in that case.
var ALLOW_MEMORY_GROWTH = 0; // If false, we abort with an error if we try to allocate more memory than
                             // we can (TOTAL_MEMORY). If true, we will grow the memory arrays at
                             // runtime, seamlessly and dynamically. This has a performance cost though,
                             // both during the actual growth and in general (the latter is because in
                             // that case we must be careful about optimizations, in particular the
                             // eliminator).
                             // See https://code.google.com/p/v8/issues/detail?id=3907 regarding
                             // memory growth performance in chrome.

var GLOBAL_BASE = -1; // where global data begins; the start of static memory. -1 means use the
                      // default, any other value will be used as an override

// Code embetterments
var MICRO_OPTS = 1; // Various micro-optimizations, like nativizing variables
var RELOOP = 0; // Recreate js native loops from llvm data
var RELOOPER = 'relooper.js'; // Loads the relooper from this path relative to compiler.js
var RELOOPER_BUFFER_SIZE = 20*1024*1024; // The internal relooper buffer size. Increase if you see assertions
                                         // on OutputBuffer.

var USE_TYPED_ARRAYS = 2; // Use typed arrays for the heap. See https://github.com/kripken/emscripten/wiki/Code-Generation-Modes/
                          // 0 means no typed arrays are used. This mode disallows LLVM optimizations
                          // 1 has two heaps, IHEAP (int32) and FHEAP (double),
                          // and addresses there are a match for normal addresses. This mode disallows LLVM optimizations.
                          // 2 is a single heap, accessible through views as int8, int32, etc. This is
                          //   the recommended mode both for performance and for compatibility.
var USE_FHEAP = 1; // Relevant in USE_TYPED_ARRAYS == 1. If this is disabled, only IHEAP will be used, and FHEAP
                   // not generated at all. This is useful if your code is 100% ints without floats or doubles
var DOUBLE_MODE = 1; // How to load and store 64-bit doubles. Without typed arrays or in typed array mode 1,
                     // this doesn't matter - these values are just values like any other. In typed array mode 2,
                     // a potential risk is that doubles may be only 32-bit aligned. Forcing 64-bit alignment
                     // in Clang itself should be able to solve that, or as a workaround in DOUBLE_MODE 1 we
                     // will carefully load in parts, in a way that requires only 32-bit alignment. In DOUBLE_MODE
                     // 0 we will simply store and load doubles as 32-bit floats, so when they are stored/loaded
                     // they will truncate from 64 to 32 bits, and lose precision. This is faster, and might
                     // work for some code (but probably that code should just use floats and not doubles anyhow).
                     // Note that a downside of DOUBLE_MODE 1 is that we currently store the double in parts,
                     // then load it aligned, and that load-store will make JS engines alter it if it is being
                     // stored to a typed array for security reasons. That will 'fix' the number from being a
                     // NaN or an infinite number.
var UNALIGNED_MEMORY = 0; // If enabled, all memory accesses are assumed to be unaligned. (This only matters in
                          // typed arrays mode 2 where alignment is relevant.) In unaligned memory mode, you
                          // can run nonportable code that typically would break in JS (or on ARM for that
                          // matter, which also cannot do unaligned reads/writes), at the cost of slowness
var FORCE_ALIGNED_MEMORY = 0; // If enabled, assumes all reads and writes are fully aligned for the type they
                              // use. This is true in proper C code (no undefined behavior), but is sadly
                              // common enough that we can't do it by default. See SAFE_HEAP and CHECK_HEAP_ALIGN
                              // for ways to help find places in your code where unaligned reads/writes are done -
                              // you might be able to refactor your codebase to prevent them, which leads to
                              // smaller and faster code, or even the option to turn this flag on.
var WARN_UNALIGNED = 0; // Warn at compile time about instructions that LLVM tells us are not fully aligned.
                        // This is useful to find places in your code where you might refactor to ensure proper
                        // alignment. (this option is fastcomp-only)
var PRECISE_I64_MATH = 1; // If enabled, i64 addition etc. is emulated - which is slow but precise. If disabled,
                          // we use the 'double trick' which is fast but incurs rounding at high values.
                          // Note that we do not catch 32-bit multiplication by default (which must be done in
                          // 64 bits for high values for full precision) - you must manually set PRECISE_I32_MUL
                          // for that.
                          // If set to 2, we always include the i64 math code, which is necessary in the case
                          // that we can't know at compile time that 64-bit math is needed. For example, if you
                          // print 64-bit values with printf, but never add them, we can't know at compile time
                          // and you need to set this to 2.
var PRECISE_I32_MUL = 1; // If enabled, i32 multiplication is done with full precision, which means it is
                         // correct even if the value exceeds the JS double-integer limit of ~52 bits (otherwise,
                         // rounding will occur above that range).
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
              // (In older versions of emscripten, in particular pre-fastcomp, SIMD=1 was needed to get
              // any SIMD output at all.)

var CLOSURE_COMPILER = 0; // Whether closure compiling is being run on this output
var CLOSURE_ANNOTATIONS = 0; // If set, the generated code will be annotated for the closure
                             // compiler. This potentially lets closure optimize the code better.

var SKIP_STACK_IN_SMALL = 1; // When enabled, does not push/pop the stack at all in
                             // functions that have no basic stack usage. But, they
                             // may allocate stack later, and in a loop, this can be
                             // very bad. In particular, when debugging, printf()ing
                             // a lot can exhaust the stack very fast, with this option.
                             // In particular, be careful with the autodebugger! (We do turn
                             // this off automatically in that case, though.)
var INLINE_LIBRARY_FUNCS = 1; // Will inline library functions that have __inline defined
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

var POINTER_MASKING = 0; // Whether pointers can be masked to a power-of-two heap
                         // length. An experimental optimization trying to reduce VM
                         // bounds checks.
var POINTER_MASKING_OVERFLOW = 64 * 1024; // The length added to the heap length to allow
                                          // the compiler to derive that accesses are
                                          // within bounds even when adding small constant
                                          // offsets. This defaults to 64K, but in asm.js
                                          // mode it is silently adjusted to keep the
                                          // total buffer length a valid asm.js heap
                                          // buffer length.
var POINTER_MASKING_DYNAMIC = 0; // When disabled, the masking is baked into the code with
				 // static masks and a static heap buffer length and the
				 // TOTAL_MEMORY must be a power of 2. When enabled, the
				 // masks are defined at runtime rather than compling them
				 // into the asm.js module as literal constants and the
				 // TOTAL_MEMORY can be defined at run time.
var POINTER_MASKING_DEFAULT_ENABLED = 1; // When POINTER_MASKING_DYNAMIC is enabled this
					 // sets the default for POINTER_MASKING_ENABLED,
					 // enabling or disabling pointer masking.

// Generated code debugging options
var SAFE_HEAP = 0; // Check each write to the heap, for example, this will give a clear
                   // error on what would be segfaults in a native build (like deferencing
                   // 0). See preamble.js for the actual checks performed.
                   // If equal to 2, done on a line-by-line basis according to
                   // SAFE_HEAP_LINES, checking only the specified lines.
                   // If equal to 3, checking all *but* the specified lines. Note
                   // that 3 is the option you usually want here.
var SAFE_HEAP_LOG = 0; // Log out all SAFE_HEAP operations

var CHECK_HEAP_ALIGN = 0; // Check heap accesses for alignment, but don't do as
                          // near extensive (or slow) checks as SAFE_HEAP.

var SAFE_DYNCALLS = 0; // Show stack traces on missing function pointer/virtual method calls

var RESERVED_FUNCTION_POINTERS = 0; // In asm.js mode, we cannot simply add function pointers to
                                    // function tables, so we reserve some slots for them.
var ALIASING_FUNCTION_POINTERS = 0; // Whether to allow function pointers to alias if they have
                                    // a different type. This can greatly decrease table sizes
                                    // in asm.js, but can break code that compares function
                                    // pointers across different types.
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

var ASM_HEAP_LOG = 0; // Simple heap logging, like SAFE_HEAP_LOG but cheaper, and in asm.js

var CORRUPTION_CHECK = 0; // When enabled, will emit a buffer area at the beginning and
                          // end of each allocation on the heap, filled with canary
                          // values that can be checked later. Corruption is checked for
                          // at the end of each at each free() (see jsifier to add more, and you
                          // can add more manual checks by calling CorruptionChecker.checkAll).
                          // 0 means not enabled, higher values mean the size of the
                          // buffer areas as a multiple of the allocated area (so
                          // 1 means 100%, or buffer areas equal to allocated area,
                          // both before and after). This must be an integer.

var LABEL_DEBUG = 0; // 1: Print out functions as we enter them
                     // 2: Also print out each label as we enter it
var LABEL_FUNCTION_FILTERS = []; // Filters for function label debug.
                                 // The items for this array will be used
                                 // as filters for function names. Only the
                                 // labels of functions that is equaled to
                                 // one of the filters are printed out
                                 // When the array is empty, the filter is disabled.
var EXCEPTION_DEBUG = 0; // Print out exceptions in emscriptened code. Does not work in asm.js mode

var DEMANGLE_SUPPORT = 0; // If 1, build in libcxxabi's full c++ demangling code, to allow stackTrace()
                          // to emit fully proper demangled c++ names

var LIBRARY_DEBUG = 0; // Print out when we enter a library call (library*.js). You can also unset
                       // Runtime.debug at runtime for logging to cease, and can set it when you
                       // want it back. A simple way to set it in C++ is
                       //   emscripten_run_script("Runtime.debug = ...;");
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
                             // in some cases, see https://github.com/kripken/emscripten/wiki/OpenGL-support
var GL_FFP_ONLY = 0; // If you specified LEGACY_GL_EMULATION = 1 and only use fixed function pipeline in your code,
                     // you can also set this to 1 to signal the GL emulation layer that it can perform extra
                     // optimizations by knowing that the user code does not use shaders at all. If 
                     // LEGACY_GL_EMULATION = 0, this setting has no effect.

var STB_IMAGE = 0; // Enables building of stb-image, a tiny public-domain library for decoding images, allowing
                   // decoding of images without using the browser's built-in decoders. The benefit is that this
                   // can be done synchronously, however, it will not be as fast as the browser itself.
                   // When enabled, stb-image will be used automatically from IMG_Load and IMG_Load_RW. You
                   // can also call the stbi_* functions directly yourself.

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
                                                                                                    

var EXECUTION_TIMEOUT = -1; // Throw an exception after X seconds - useful to debug infinite loops
var CHECK_OVERFLOWS = 0; // Add code that checks for overflows in integer math operations.
                         // There is currently not much to do to handle overflows if they occur.
                         // We can add code to simulate i32/i64 overflows in JS, but that would
                         // be very slow. It probably makes more sense to avoid overflows in
                         // C/C++ code. For example, if you have an int that you multiply by
                         // some factor, in order to get 'random' hash values - by taking
                         // that |value & hash_table_size| - then multiplying enough times will overflow.
                         // But instead, you can do |value = value & 30_BITS| in each iteration.
var CHECK_SIGNED_OVERFLOWS = 0; // Whether to allow *signed* overflows - our correction for overflows generates signed
                                // values (since we use &). This means that we correct some things are not strictly overflows,
                                // and we cause them to be signed (which may lead to unnecessary unSign()ing later).
                                // With this enabled, we check signed overflows for CHECK_OVERFLOWS
var CORRECT_OVERFLOWS = 1; // Experimental code that tries to prevent unexpected JS overflows in integer
                           // mathops, by doing controlled overflows (sort of parallel to a CPU).
                           // Note that as mentioned above in CHECK_OVERFLOWS, the best thing is to
                           // not rely on overflows in your C/C++ code, as even if this option works,
                           // it slows things down.
                           //
                           // If equal to 2, done on a line-by-line basis according to
                           // CORRECT_OVERFLOWS_LINES, correcting only the specified lines.
                           // If equal to 3, correcting all *but* the specified lines
                           //
                           // NOTE: You can introduce signing issues by using this option. If you
                           //       take a large enough 32-bit value, and correct it for overflows,
                           //       you may get a negative number, as JS & operations are signed.
var CORRECT_ROUNDINGS = 1; // C rounds to 0 (-5.5 to -5, +5.5 to 5), while JS has no direct way to do that:
                           // Math.floor is to negative, ceil to positive. With CORRECT_ROUNDINGS,
                           // we will do slow but correct C rounding operations.
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
var NO_BROWSER = 0; // If set, disables building in browser support using the Browser object. Useful if you are
                    // just doing pure computation in a library, and don't need any browser capabilities like a main loop
                    // (emscripten_set_main_loop), or setTimeout, etc.

var USE_BSS = 1; // https://en.wikipedia.org/wiki/.bss
                 // When enabled, 0-initialized globals are sorted to the end of the globals list,
                 // enabling us to not explicitly store the initialization value for each 0 byte.
                 // This significantly lowers the memory initialization array size.

var NAMED_GLOBALS = 0; // If 1, we use global variables for globals. Otherwise
                       // they are referred to by a base plus an offset (called an indexed global),
                       // saving global variables but adding runtime overhead.

var NODE_STDOUT_FLUSH_WORKAROUND = 1; // Whether or not to work around node issues with not flushing stdout. This
                                      // can cause unnecessary whitespace to be printed.

var EXPORTED_FUNCTIONS = ['_main', '_malloc'];
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


// JS library functions (C functions implemented in JS)
// that we include by default. If you want to make sure
// something is included by the JS compiler, add it here.
// For example, if you do not use some emscripten_*
// C API call from C, but you want to call it from JS,
// add it here (and in EXPORTED FUNCTIONS with prefix
// "_", for closure).
var DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = ['memcpy', 'memset', 'malloc', 'free', 'strlen', '$Browser'];

var LIBRARY_DEPS_TO_AUTOEXPORT = ['memcpy']; // This list is also used to determine
                                             // auto-exporting of library dependencies (i.e., functions that
                                             // might be dependencies of JS library functions, that if
                                             // so we must export so that if they are implemented in C
                                             // they will be accessible, in ASM_JS mode).

var IGNORED_FUNCTIONS = []; // Functions that we should not generate, neither a stub nor a complete function.
                            // This is useful if your project code includes a function, and you want to replace
                            // that in the compiled code with your own handwritten JS. (Of course even without
                            // this option, you could just override the generated function at runtime. However,
                            // JS engines might optimize better if the function is defined once in a single
                            // place in your code.)

var EXPORTED_GLOBALS = []; // Global non-function variables that are explicitly
                           // exported, so they are guaranteed to be
                           // accessible outside of the generated code.

var INCLUDE_FULL_LIBRARY = 0; // Whether to include the whole library rather than just the
                              // functions used by the generated code. This is needed when
                              // dynamically loading modules that make use of runtime
                              // library functions that are not used in the main module.
                              // Note that this includes js libraries but *not* C. You will
                              // need the main file to include all needed C libraries. For
                              // example, if a library uses malloc or new, you will need
                              // to use those in the main file too to link in dlmalloc.

var SHELL_FILE = 0; // set this to a string to override the shell file used

var SHOW_LABELS = 0; // Show labels in the generated code

var PRINT_SPLIT_FILE_MARKER = 0; // Prints markers in Javascript generation to split the file later on. See emcc --split option.

var MAIN_MODULE = 0; // A main module is a file compiled in a way that allows us to link it to
                     // a side module using emlink.py.
var SIDE_MODULE = 0; // Corresponds to MAIN_MODULE

var BUILD_AS_SHARED_LIB = 0; // Whether to build the code as a shared library
                             // 0 here means this is not a shared lib: It is a main file.
                             // 1 means this is a normal shared lib, load it with dlopen()
                             // 2 means this is a shared lib that will be linked at runtime,
                             //   which means it will insert its functions into
                             //   the global namespace. See STATIC_LIBS_TO_LOAD.
                             //
                             // Value 2 is currently deprecated.
var RUNTIME_LINKED_LIBS = []; // If this is a main file (BUILD_AS_SHARED_LIB == 0), then
                              // we will link these at runtime. They must have been built with
                              // BUILD_AS_SHARED_LIB == 2.
                              // NOTE: LLVM optimizations run separately on the main file and
                              //       linked libraries can break things.
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

var DLOPEN_SUPPORT = 0; // Full support for dlopen. This is necessary for asm.js and for all code
                        // modes for dlopen(NULL, ...). Note that you must use EMSCRIPTEN_KEEPALIVE
                        // to ensure that functions and globals can be accessed through dlsym,
                        // otherwise LLVM may optimize them out.

var RUNTIME_TYPE_INFO = 0; // Whether to expose type info to the script at run time. This
                           // increases the size of the generated script, but allows you
                           // to more easily perform operations from handwritten JS on
                           // objects with structures etc.

var FAKE_X86_FP80 = 1; // Replaces x86_fp80 with double. This loses precision. It is better,
                       // if you can, to get the original source code to build without x86_fp80
                       // (which is nonportable anyhow).

var GC_SUPPORT = 1; // Enables GC, see gc.h (this does not add overhead, so it is on by default)

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

var BENCHMARK = 0; // If 1, will just time how long main() takes to execute, and not
                   // print out anything at all whatsoever. This is useful for benchmarking.

var ASM_JS = 1; // If 1, generate code in asm.js format. If 2, emits the same code except
                // for omitting 'use asm'
var FINALIZE_ASM_JS = 1; // If 1, will finalize the final emitted code, including operations
                         // that prevent later js optimizer passes from running, like
                         // converting +5 into 5.0 (the js optimizer sees 5.0 as just 5).

var SWAPPABLE_ASM_MODULE = 0; // If 1, then all exports from the asm.js module will be accessed
                              // indirectly, which allow the asm module to be swapped later.

var PGO = 0; // Enables profile-guided optimization in the form of runtime checks for
             // which functions are actually called. Emits a list during shutdown that you
             // can pass to DEAD_FUNCTIONS (you can also emit the list manually by
             // calling PGOMonitor.dump());
var DEAD_FUNCTIONS = []; // Functions on this list are not converted to JS, and calls to
                         // them are turned into abort()s. This is potentially useful for
                         // reducing code size.
                         // If a dead function is actually called, you will get a runtime
                         // error.
                         // This can affect both functions in compiled code, and system
                         // library functions (e.g., you can use this to kill printf).
                         // TODO: options to lazily load such functions

var EXPLICIT_ZEXT = 0; // If 1, generate an explicit conversion of zext i1 to i32, using ?:

var NECESSARY_BLOCKADDRS = []; // List of (function, block) for all block addresses that are taken.

var JS_CHUNK_SIZE = 10240; // Used as a maximum size before breaking up expressions and lines into smaller pieces

var EXPORT_NAME = 'Module'; // Global variable to export the module as for environments without a standardized module
                            // loading system (e.g. the browser and SM shell).

var NO_DYNAMIC_EXECUTION = 0; // When enabled, we do not emit eval() and new Function(), which disables some functionality
                              // (causing runtime errors if attempted to be used), but allows the emitted code to be
                              // acceptable in places that disallow dynamic code execution (chrome packaged app, non-
                              // privileged firefox app, etc.)

var EMTERPRETIFY = 0; // Runs tools/emterpretify on the compiler output
var EMTERPRETIFY_BLACKLIST = []; // Functions to not emterpret, that is, to run normally at full speed
var EMTERPRETIFY_WHITELIST = []; // If this contains any functions, then only the functions in this list
                                 // are emterpreted (as if all the rest are blacklisted; this overrides the BLACKLIST)
var EMTERPRETIFY_YIELDLIST = []; // A list of functions that are allowed to run during while sleeping. Typically this is
                                 // during  emscripten_sleep_with_yield  , but also you may need to add methods to this list
                                 // for things like event handling (an SDL EventHandler will be called from the event, directly -
                                 // if we do that later, you lose out on the whole point of an EventHandler, which is to let
                                 // you react to key presses in order to launch fullscreen, etc.).
                                 // Functions in the yield list do not trigger asserts checking on running during a sleep,
                                 // in ASSERTIONS builds, 
var EMTERPRETIFY_ASYNC = 0; // Allows sync code in the emterpreter, by saving the call stack, doing an async delay, and resuming it
var EMTERPRETIFY_ADVISE = 0; // Performs a static analysis to suggest which functions should be run in the emterpreter, as it
                             // appears they can be on the stack when a sync function is called in the EMTERPRETIFY_ASYNC option.
                             // After showing the suggested list, compilation will halt. You can apply the provided list as an
                             // emcc argument when compiling later.
                             // This will also advise on the YIELDLIST, if it contains at least one value (it then reports
                             // all things reachable from that function, as they may need to be in the YIELDLIST as well).
                             // Note that this depends on things like inlining. If you run this with different inlining than
                             // when you use the list, it might not work.

var RUNNING_JS_OPTS = 0; // whether js opts will be run, after the main compiler
var RUNNING_FASTCOMP = 1; // whether we are running the fastcomp backend
var BOOTSTRAPPING_STRUCT_INFO = 0; // whether we are in the generate struct_info bootstrap phase

var COMPILER_ASSERTIONS = 0; // costly (slow) compile-time assertions
var COMPILER_FASTPATHS = 1; // use fast-paths to speed up compilation

var EMSCRIPTEN_TRACING = 0; // Add some calls to emscripten tracing APIs

var USE_GLFW = 2; // Specify the GLFW version that is being linked against.
                  // Only relevant, if you are linking against the GLFW library.
                  // Valid options are 2 for GLFW2 and 3 for GLFW3.

// Ports

var USE_SDL = 1; // Specify the SDL version that is being linked against.
                 // 1, the default, is 1.3, which is implemented in JS
                 // 2 is a port of the SDL C code on emscripten-ports
var USE_SDL_IMAGE = 1; // Specify the SDL_image version that is being linked against. Must match USE_SDL
var USE_ZLIB = 0; // 1 = use zlib from emscripten-ports


// Compiler debugging options
var DEBUG_TAGS_SHOWING = [];
  // Some useful items:
  //    framework
  //    frameworkLines
  //    gconst
  //    types
  //    vars
  //    relooping
  //    unparsedFunctions
  //    metadata
  //    legalizer

// For internal use only
var ORIGINAL_EXPORTED_FUNCTIONS = [];
var CORRECT_OVERFLOWS_LINES = [];
var CORRECT_SIGNS_LINES = [];
var CORRECT_ROUNDINGS_LINES = [];
var SAFE_HEAP_LINES = [];

// The list of defines (C_DEFINES) was moved into struct_info.json in the same directory.
// That file is automatically parsed by tools/gen_struct_info.py.
// If you modify the headers, just clear your cache and emscripten libc should see
// the new values.
