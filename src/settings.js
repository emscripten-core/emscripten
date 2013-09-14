
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
var TARGET_LE32 = 1; // For le32-unknown-nacl

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
                    // exceed it's size, whether all allocations (stack and static) are
                    // of positive size, etc., whether we should throw if we encounter a bad __label__, i.e.,
                    // if code flow runs into a fault
                    // ASSERTIONS == 2 gives even more runtime checks
var VERBOSE = 0; // When set to 1, will generate more verbose output during compilation.

var INVOKE_RUN = 1; // Whether we will run the main() function. Disable if you embed the generated
                    // code in your own, and will call main() yourself at the right time (which you
                    // can do with Module.callMain(), with an optional parameter of commandline args).
var INIT_HEAP = 0; // Whether to initialize memory anywhere other than the stack to 0.
var TOTAL_STACK = 5*1024*1024; // The total stack size. There is no way to enlarge the stack, so this
                               // value must be large enough for the program's requirements. If
                               // assertions are on, we will assert on not exceeding this, otherwise,
                               // it will fail silently.
var TOTAL_MEMORY = 16777216;     // The total amount of memory to use. Using more memory than this will
                                 // cause us to expand the heap, which can be costly with typed arrays:
                                 // we need to copy the old heap into a new one in that case.
var FAST_MEMORY = 2*1024*1024; // The amount of memory to initialize to 0. This ensures it will be
                               // in a flat array. This only matters in non-typed array builds.
var ALLOW_MEMORY_GROWTH = 0; // If false, we abort with an error if we try to allocate more memory than
                             // we can (TOTAL_MEMORY). If true, we will grow the memory arrays at
                             // runtime, seamlessly and dynamically. This has a performance cost though,
                             // both during the actual growth and in general (the latter is because in
                             // that case we must be careful about optimizations, in particular the
                             // eliminator). Note that memory growth is only supported with typed
                             // arrays.
var MAX_SETJMPS = 20; // size of setjmp table allocated in each function invocation (that has setjmp)

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
var TO_FLOAT32 = 0; // Use Math.toFloat32

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

var LIBRARY_DEBUG = 0; // Print out when we enter a library call (library*.js). You can also unset
                       // Runtime.debug at runtime for logging to cease, and can set it when you
                       // want it back. A simple way to set it in C++ is
                       //   emscripten_run_script("Runtime.debug = ...;");
var SOCKET_DEBUG = 0; // Log out socket/network data transfer.
var SOCKET_WEBRTC = 0; // Select socket backend, either webrtc or websockets.

var OPENAL_DEBUG = 0; // Print out debugging information from our OpenAL implementation.

var GL_DEBUG = 0; // Print out all calls into WebGL. As with LIBRARY_DEBUG, you can set a runtime
                  // option, in this case GL.debug.
var GL_TESTING = 0; // When enabled, sets preserveDrawingBuffer in the context, to allow tests to work (but adds overhead)
var GL_MAX_TEMP_BUFFER_SIZE = 2097152; // How large GL emulation temp buffers are
var GL_UNSAFE_OPTS = 1; // Enables some potentially-unsafe optimizations in GL emulation code
var FULL_ES2 = 0; // Forces support for all GLES2 features, not just the WebGL-friendly subset.
var LEGACY_GL_EMULATION = 0; // Includes code to emulate various desktop GL features. Incomplete but useful
                             // in some cases, see https://github.com/kripken/emscripten/wiki/OpenGL-support

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

var EXCEPTION_CATCHING_WHITELIST = [];  // Enables catching exception in listed functions if
                                        // DISABLE_EXCEPTION_CATCHING = 2 set

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

var USE_BSS = 1; // https://en.wikipedia.org/wiki/.bss
                 // When enabled, 0-initialized globals are sorted to the end of the globals list,
                 // enabling us to not explicitly store the initialization value for each 0 byte.
                 // This significantly lowers the memory initialization array size.

var NAMED_GLOBALS = 0; // If 1, we use global variables for globals. Otherwise
                       // they are referred to by a base plus an offset (called an indexed global),
                       // saving global variables but adding runtime overhead.

var EXPORTED_FUNCTIONS = ['_main', '_malloc'];
                                    // Functions that are explicitly exported. These functions are kept alive
                                    // through LLVM dead code elimination, and also made accessible outside of
                                    // the generated code even after running closure compiler (on "Module").
                                    // Note the necessary prefix of "_".
var EXPORT_ALL = 0; // If true, we export all the symbols. Note that this does *not* affect LLVM, so it can
                    // still eliminate functions as dead. This just exports them on the Module object.
var EXPORT_BINDINGS = 0; // Export all bindings generator functions (prefixed with emscripten_bind_). This
                         // is necessary to use the bindings generator with asm.js

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

var WARN_ON_UNDEFINED_SYMBOLS = 0; // If set to 1, we will warn on any undefined symbols that
                                   // are not resolved by the library_*.js files. We by default
                                   // do not warn because (1) it is normal in large projects to
                                   // not implement everything, when you know what is not
                                   // going to actually be called (and don't want to mess with
                                   // the existing buildsystem), and (2) functions might be
                                   // implemented later on, say in --pre-js

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

var BENCHMARK = 0; // If 1, will just time how long main() takes to execute, and not
                   // print out anything at all whatsoever. This is useful for benchmarking.

var ASM_JS = 0; // If 1, generate code in asm.js format.

var PGO = 0; // Enables profile-guided optimization in the form of runtime checks for
             // which functions are actually called. Emits a list during shutdown that you
             // can pass to DEAD_FUNCTIONS (you can also emit the list manually by
             // calling PGOMonitor.dump());
var DEAD_FUNCTIONS = []; // Functions on this list are not converted to JS, and calls to
                         // them are turned into abort()s. This is potentially useful for
                         // reducing code size.
                         // If a dead function is actually called, you will get a runtime
                         // error.
                         // TODO: options to lazily load such functions

var EXPLICIT_ZEXT = 0; // If 1, generate an explicit conversion of zext i1 to i32, using ?:

var NECESSARY_BLOCKADDRS = []; // List of (function, block) for all block addresses that are taken.

var EMIT_GENERATED_FUNCTIONS = 0; // whether to emit the list of generated functions, needed for external JS optimization passes

var JS_CHUNK_SIZE = 10240; // Used as a maximum size before breaking up expressions and lines into smaller pieces

var EXPORT_NAME = 'Module'; // Global variable to export the module as for environments without a standardized module
                            // loading system (e.g. the browser and SM shell).

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

// A cached set of defines, generated from the header files. This
// lets the emscripten libc (library.js) see the right values.
// If you modify the headers or use different ones, you will need
// to override this.
var C_DEFINES = {
    'ABDAY_1': '131072',
    'ABDAY_2': '131073',
    'ABDAY_3': '131074',
    'ABDAY_4': '131075',
    'ABDAY_5': '131076',
    'ABDAY_6': '131077',
    'ABDAY_7': '131078',
    'ABMON_1': '131086',
    'ABMON_10': '131095',
    'ABMON_11': '131096',
    'ABMON_12': '131097',
    'ABMON_2': '131087',
    'ABMON_3': '131088',
    'ABMON_4': '131089',
    'ABMON_5': '131090',
    'ABMON_6': '131091',
    'ABMON_7': '131092',
    'ABMON_8': '131093',
    'ABMON_9': '131094',
    'AF_INET': '2',
    'AF_INET6': '10',
    'AF_UNSPEC': '0',
    'AI_ADDRCONFIG': '32',
    'AI_ALL': '16',
    'AI_CANONNAME': '2',
    'AI_NUMERICHOST': '4',
    'AI_NUMERICSERV': '1024',
    'AI_PASSIVE': '1',
    'AI_V4MAPPED': '8',
    'ALT_DIGITS': '131119',
    'AM_STR': '131110',
    'CLOCKS_PER_SEC': '1000000',
    'CODESET': '14',
    'CRNCYSTR': '262159',
    'DAY_1': '131079',
    'DAY_2': '131080',
    'DAY_3': '131081',
    'DAY_4': '131082',
    'DAY_5': '131083',
    'DAY_6': '131084',
    'DAY_7': '131085',
    'D_FMT': '131113',
    'D_T_FMT': '131112',
    'E2BIG': '7',
    'EACCES': '13',
    'EADDRINUSE': '98',
    'EADDRNOTAVAIL': '99',
    'EADV': '68',
    'EAFNOSUPPORT': '97',
    'EAGAIN': '11',
    'EAI_BADFLAGS': '-1',
    'EAI_FAMILY': '-6',
    'EAI_NONAME': '-2',
    'EAI_OVERFLOW': '-12',
    'EAI_SERVICE': '-8',
    'EAI_SOCKTYPE': '-7',
    'EALREADY': '114',
    'EBADE': '52',
    'EBADF': '9',
    'EBADFD': '77',
    'EBADMSG': '74',
    'EBADR': '53',
    'EBADRQC': '56',
    'EBADSLT': '57',
    'EBFONT': '59',
    'EBUSY': '16',
    'ECANCELED': '125',
    'ECHILD': '10',
    'ECHRNG': '44',
    'ECOMM': '70',
    'ECONNABORTED': '103',
    'ECONNREFUSED': '111',
    'ECONNRESET': '104',
    'EDEADLK': '35',
    'EDEADLOCK': '35',
    'EDESTADDRREQ': '89',
    'EDOM': '33',
    'EDOTDOT': '73',
    'EDQUOT': '122',
    'EEXIST': '17',
    'EFAULT': '14',
    'EFBIG': '27',
    'EHOSTDOWN': '112',
    'EHOSTUNREACH': '113',
    'EIDRM': '43',
    'EILSEQ': '84',
    'EINPROGRESS': '115',
    'EINTR': '4',
    'EINVAL': '22',
    'EIO': '5',
    'EISCONN': '106',
    'EISDIR': '21',
    'EL2HLT': '51',
    'EL2NSYNC': '45',
    'EL3HLT': '46',
    'EL3RST': '47',
    'ELIBACC': '79',
    'ELIBBAD': '80',
    'ELIBEXEC': '83',
    'ELIBMAX': '82',
    'ELIBSCN': '81',
    'ELNRNG': '48',
    'ELOOP': '40',
    'EMFILE': '24',
    'EMLINK': '31',
    'EMSGSIZE': '90',
    'EMULTIHOP': '72',
    'ENAMETOOLONG': '36',
    'ENETDOWN': '100',
    'ENETRESET': '102',
    'ENETUNREACH': '101',
    'ENFILE': '23',
    'ENOANO': '55',
    'ENOBUFS': '105',
    'ENOCSI': '50',
    'ENODATA': '61',
    'ENODEV': '19',
    'ENOENT': '2',
    'ENOEXEC': '8',
    'ENOLCK': '37',
    'ENOLINK': '67',
    'ENOMEDIUM': '123',
    'ENOMEM': '12',
    'ENOMSG': '42',
    'ENONET': '64',
    'ENOPKG': '65',
    'ENOPROTOOPT': '92',
    'ENOSPC': '28',
    'ENOSR': '63',
    'ENOSTR': '60',
    'ENOSYS': '38',
    'ENOTBLK': '15',
    'ENOTCONN': '107',
    'ENOTDIR': '20',
    'ENOTEMPTY': '39',
    'ENOTRECOVERABLE': '131',
    'ENOTSOCK': '88',
    'ENOTSUP': '95',
    'ENOTTY': '25',
    'ENOTUNIQ': '76',
    'ENXIO': '6',
    'EOF': '-1',
    'EOPNOTSUPP': '95',
    'EOVERFLOW': '75',
    'EOWNERDEAD': '130',
    'EPERM': '1',
    'EPFNOSUPPORT': '96',
    'EPIPE': '32',
    'EPROTO': '71',
    'EPROTONOSUPPORT': '93',
    'EPROTOTYPE': '91',
    'ERA': '131116',
    'ERANGE': '34',
    'ERA_D_FMT': '131118',
    'ERA_D_T_FMT': '131120',
    'ERA_T_FMT': '131121',
    'EREMCHG': '78',
    'EREMOTE': '66',
    'EROFS': '30',
    'ESHUTDOWN': '108',
    'ESOCKTNOSUPPORT': '94',
    'ESPIPE': '29',
    'ESRCH': '3',
    'ESRMNT': '69',
    'ESTALE': '116',
    'ESTRPIPE': '86',
    'ETIME': '62',
    'ETIMEDOUT': '110',
    'ETOOMANYREFS': '109',
    'ETXTBSY': '26',
    'EUNATCH': '49',
    'EUSERS': '87',
    'EWOULDBLOCK': '11',
    'EXDEV': '18',
    'EXFULL': '54',
    'FIONREAD': '21531',
    'FP_INFINITE': '1',
    'FP_NAN': '0',
    'FP_NORMAL': '4',
    'FP_ZERO': '2',
    'F_DUPFD': '0',
    'F_GETFD': '1',
    'F_GETFL': '3',
    'F_GETLK': '12',
    'F_GETLK64': '12',
    'F_GETOWN': '9',
    'F_SETFD': '2',
    'F_SETFL': '4',
    'F_SETLK': '13',
    'F_SETLK64': '13',
    'F_SETLKW': '14',
    'F_SETLKW64': '14',
    'F_SETOWN': '8',
    'F_UNLCK': '2',
    'INADDR_LOOPBACK': '2130706433',
    'IPPROTO_TCP': '6',
    'IPPROTO_UDP': '17',
    'MAP_PRIVATE': '2',
    'MON_1': '131098',
    'MON_10': '131107',
    'MON_11': '131108',
    'MON_12': '131109',
    'MON_2': '131099',
    'MON_3': '131100',
    'MON_4': '131101',
    'MON_5': '131102',
    'MON_6': '131103',
    'MON_7': '131104',
    'MON_8': '131105',
    'MON_9': '131106',
    'NI_NAMEREQD': '8',
    'NI_NUMERICHOST': '1',
    'NOEXPR': '327681',
    'O_ACCMODE': '2097155',
    'O_APPEND': '1024',
    'O_CREAT': '64',
    'O_EXCL': '128',
    'O_NOFOLLOW': '131072',
    'O_RDONLY': '0',
    'O_RDWR': '2',
    'O_SYNC': '1052672',
    'O_TRUNC': '512',
    'O_WRONLY': '1',
    'PM_STR': '131111',
    'POLLERR': '8',
    'POLLHUP': '16',
    'POLLIN': '1',
    'POLLNVAL': '32',
    'POLLOUT': '4',
    'POLLPRI': '2',
    'POLLRDNORM': '64',
    'RADIXCHAR': '65536',
    'R_OK': '4',
    'SEEK_END': '2',
    'SEEK_SET': '0',
    'SOCK_DGRAM': '2',
    'SOCK_STREAM': '1',
    'S_IALLUGO': '4095',
    'S_IFBLK': '24576',
    'S_IFCHR': '8192',
    'S_IFDIR': '16384',
    'S_IFIFO': '4096',
    'S_IFLNK': '40960',
    'S_IFMT': '61440',
    'S_IFREG': '32768',
    'S_IFSOCK': '49152',
    'S_IRUGO': '292',
    'S_IRWXO': '7',
    'S_IRWXUGO': '511',
    'S_ISVTX': '512',
    'S_IWUGO': '146',
    'S_IXUGO': '73',
    'THOUSEP': '65537',
    'T_FMT': '131114',
    'T_FMT_AMPM': '131115',
    'W_OK': '2',
    'X_OK': '1',
    'YESEXPR': '327680',
    '_CS_GNU_LIBC_VERSION': '2',
    '_CS_GNU_LIBPTHREAD_VERSION': '3',
    '_CS_PATH': '0',
    '_CS_POSIX_V6_ILP32_OFF32_CFLAGS': '1116',
    '_CS_POSIX_V6_ILP32_OFF32_LDFLAGS': '1117',
    '_CS_POSIX_V6_ILP32_OFF32_LIBS': '1118',
    '_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS': '1120',
    '_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS': '1121',
    '_CS_POSIX_V6_ILP32_OFFBIG_LIBS': '1122',
    '_CS_POSIX_V6_LP64_OFF64_CFLAGS': '1124',
    '_CS_POSIX_V6_LP64_OFF64_LDFLAGS': '1125',
    '_CS_POSIX_V6_LP64_OFF64_LIBS': '1126',
    '_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS': '1128',
    '_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS': '1129',
    '_CS_POSIX_V6_LPBIG_OFFBIG_LIBS': '1130',
    '_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS': '1',
    '_PC_2_SYMLINKS': '20',
    '_PC_ALLOC_SIZE_MIN': '18',
    '_PC_ASYNC_IO': '10',
    '_PC_CHOWN_RESTRICTED': '6',
    '_PC_FILESIZEBITS': '13',
    '_PC_LINK_MAX': '0',
    '_PC_MAX_CANON': '1',
    '_PC_MAX_INPUT': '2',
    '_PC_NAME_MAX': '3',
    '_PC_NO_TRUNC': '7',
    '_PC_PATH_MAX': '4',
    '_PC_PIPE_BUF': '5',
    '_PC_PRIO_IO': '11',
    '_PC_REC_INCR_XFER_SIZE': '14',
    '_PC_REC_MAX_XFER_SIZE': '15',
    '_PC_REC_MIN_XFER_SIZE': '16',
    '_PC_REC_XFER_ALIGN': '17',
    '_PC_SOCK_MAXBUF': '12',
    '_PC_SYMLINK_MAX': '19',
    '_PC_SYNC_IO': '9',
    '_PC_VDISABLE': '8',
    '_SC_2_CHAR_TERM': '95',
    '_SC_2_C_BIND': '47',
    '_SC_2_C_DEV': '48',
    '_SC_2_FORT_DEV': '49',
    '_SC_2_FORT_RUN': '50',
    '_SC_2_LOCALEDEF': '52',
    '_SC_2_PBS': '168',
    '_SC_2_PBS_ACCOUNTING': '169',
    '_SC_2_PBS_CHECKPOINT': '175',
    '_SC_2_PBS_LOCATE': '170',
    '_SC_2_PBS_MESSAGE': '171',
    '_SC_2_PBS_TRACK': '172',
    '_SC_2_SW_DEV': '51',
    '_SC_2_UPE': '97',
    '_SC_2_VERSION': '46',
    '_SC_ADVISORY_INFO': '132',
    '_SC_AIO_LISTIO_MAX': '23',
    '_SC_AIO_MAX': '24',
    '_SC_AIO_PRIO_DELTA_MAX': '25',
    '_SC_ARG_MAX': '0',
    '_SC_ASYNCHRONOUS_IO': '12',
    '_SC_ATEXIT_MAX': '87',
    '_SC_BARRIERS': '133',
    '_SC_BC_BASE_MAX': '36',
    '_SC_BC_DIM_MAX': '37',
    '_SC_BC_SCALE_MAX': '38',
    '_SC_BC_STRING_MAX': '39',
    '_SC_CHILD_MAX': '1',
    '_SC_CLK_TCK': '2',
    '_SC_CLOCK_SELECTION': '137',
    '_SC_COLL_WEIGHTS_MAX': '40',
    '_SC_CPUTIME': '138',
    '_SC_DELAYTIMER_MAX': '26',
    '_SC_EXPR_NEST_MAX': '42',
    '_SC_FSYNC': '15',
    '_SC_GETGR_R_SIZE_MAX': '69',
    '_SC_GETPW_R_SIZE_MAX': '70',
    '_SC_HOST_NAME_MAX': '180',
    '_SC_IOV_MAX': '60',
    '_SC_IPV6': '235',
    '_SC_JOB_CONTROL': '7',
    '_SC_LINE_MAX': '43',
    '_SC_LOGIN_NAME_MAX': '71',
    '_SC_MAPPED_FILES': '16',
    '_SC_MEMLOCK': '17',
    '_SC_MEMLOCK_RANGE': '18',
    '_SC_MEMORY_PROTECTION': '19',
    '_SC_MESSAGE_PASSING': '20',
    '_SC_MONOTONIC_CLOCK': '149',
    '_SC_MQ_OPEN_MAX': '27',
    '_SC_MQ_PRIO_MAX': '28',
    '_SC_NGROUPS_MAX': '3',
    '_SC_NPROCESSORS_ONLN': '84',
    '_SC_OPEN_MAX': '4',
    '_SC_PAGE_SIZE': '30',
    '_SC_PRIORITIZED_IO': '13',
    '_SC_PRIORITY_SCHEDULING': '10',
    '_SC_RAW_SOCKETS': '236',
    '_SC_READER_WRITER_LOCKS': '153',
    '_SC_REALTIME_SIGNALS': '9',
    '_SC_REGEXP': '155',
    '_SC_RE_DUP_MAX': '44',
    '_SC_RTSIG_MAX': '31',
    '_SC_SAVED_IDS': '8',
    '_SC_SEMAPHORES': '21',
    '_SC_SEM_NSEMS_MAX': '32',
    '_SC_SEM_VALUE_MAX': '33',
    '_SC_SHARED_MEMORY_OBJECTS': '22',
    '_SC_SHELL': '157',
    '_SC_SIGQUEUE_MAX': '34',
    '_SC_SPAWN': '159',
    '_SC_SPIN_LOCKS': '154',
    '_SC_SPORADIC_SERVER': '160',
    '_SC_STREAM_MAX': '5',
    '_SC_SYMLOOP_MAX': '173',
    '_SC_SYNCHRONIZED_IO': '14',
    '_SC_THREADS': '67',
    '_SC_THREAD_ATTR_STACKADDR': '77',
    '_SC_THREAD_ATTR_STACKSIZE': '78',
    '_SC_THREAD_CPUTIME': '139',
    '_SC_THREAD_DESTRUCTOR_ITERATIONS': '73',
    '_SC_THREAD_KEYS_MAX': '74',
    '_SC_THREAD_PRIORITY_SCHEDULING': '79',
    '_SC_THREAD_PRIO_INHERIT': '80',
    '_SC_THREAD_PRIO_PROTECT': '81',
    '_SC_THREAD_PROCESS_SHARED': '82',
    '_SC_THREAD_SAFE_FUNCTIONS': '68',
    '_SC_THREAD_SPORADIC_SERVER': '161',
    '_SC_THREAD_STACK_MIN': '75',
    '_SC_THREAD_THREADS_MAX': '76',
    '_SC_TIMEOUTS': '164',
    '_SC_TIMERS': '11',
    '_SC_TIMER_MAX': '35',
    '_SC_TRACE': '181',
    '_SC_TRACE_EVENT_FILTER': '182',
    '_SC_TRACE_EVENT_NAME_MAX': '242',
    '_SC_TRACE_INHERIT': '183',
    '_SC_TRACE_LOG': '184',
    '_SC_TRACE_NAME_MAX': '243',
    '_SC_TRACE_SYS_MAX': '244',
    '_SC_TRACE_USER_EVENT_MAX': '245',
    '_SC_TTY_NAME_MAX': '72',
    '_SC_TYPED_MEMORY_OBJECTS': '165',
    '_SC_TZNAME_MAX': '6',
    '_SC_V6_ILP32_OFF32': '176',
    '_SC_V6_ILP32_OFFBIG': '177',
    '_SC_V6_LP64_OFF64': '178',
    '_SC_V6_LPBIG_OFFBIG': '179',
    '_SC_VERSION': '29',
    '_SC_XBS5_ILP32_OFF32': '125',
    '_SC_XBS5_ILP32_OFFBIG': '126',
    '_SC_XBS5_LP64_OFF64': '127',
    '_SC_XBS5_LPBIG_OFFBIG': '128',
    '_SC_XOPEN_CRYPT': '92',
    '_SC_XOPEN_ENH_I18N': '93',
    '_SC_XOPEN_LEGACY': '129',
    '_SC_XOPEN_REALTIME': '130',
    '_SC_XOPEN_REALTIME_THREADS': '131',
    '_SC_XOPEN_SHM': '94',
    '_SC_XOPEN_STREAMS': '246',
    '_SC_XOPEN_UNIX': '91',
    '_SC_XOPEN_VERSION': '89'
};

