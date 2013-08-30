
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

var DLOPEN_SUPPORT = 0; // Whether to support dlopen(NULL, ...) which enables dynamic access to the
                        // module's functions and globals. Implies LINKABLE=1, because we do not want
                        // dead code elimination.

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
   'ABDAY_1': '14',
   'ABDAY_2': '15',
   'ABDAY_3': '16',
   'ABDAY_4': '17',
   'ABDAY_5': '18',
   'ABDAY_6': '19',
   'ABDAY_7': '20',
   'ABMON_1': '33',
   'ABMON_10': '42',
   'ABMON_11': '43',
   'ABMON_12': '44',
   'ABMON_2': '34',
   'ABMON_3': '35',
   'ABMON_4': '36',
   'ABMON_5': '37',
   'ABMON_6': '38',
   'ABMON_7': '39',
   'ABMON_8': '40',
   'ABMON_9': '41',
   'ACCESSPERMS': '0000400',
   'AF_UNSPEC': '0',
   'AF_INET': '2',
   'AF_INET6': '10',
   'ALLPERMS': '0004000',
   'ALT_DIGITS': '49',
   'AM_STR': '5',
   'ARG_MAX': '4096',
   'AT_EACCESS': '1',
   'AT_FDCWD': '-2',
   'AT_REMOVEDIR': '8',
   'AT_SYMLINK_FOLLOW': '4',
   'AT_SYMLINK_NOFOLLOW': '2',
   'CHAR_BIT': '8',
   'CHAR_MAX': '127',
   'CHAR_MIN': '-128',
   'CLK_TCK': '1000',
   'CLOCKS_PER_SEC': '1000',
   'CLOCK_ALLOWED': '1',
   'CLOCK_DISABLED': '0',
   'CLOCK_DISALLOWED': '0',
   'CLOCK_ENABLED': '1',
   'CODESET': '0',
   'CRNCYSTR': '56',
   'DAY_1': '7',
   'DAY_2': '8',
   'DAY_3': '9',
   'DAY_4': '10',
   'DAY_5': '11',
   'DAY_6': '12',
   'DAY_7': '13',
   'DEFFILEMODE': '0000400',
   'DOMAIN': '1',
   'D_FMT': '2',
   'D_MD_ORDER': '57',
   'D_T_FMT': '1',
   'EOF': '-1',
   'ERA': '45',
   'ERA_D_FMT': '46',
   'ERA_D_T_FMT': '47',
   'ERA_T_FMT': '48',
   'FAPPEND': '8',
   'FASYNC': '64',
   'FCREAT': '512',
   'FDEFER': '32',
   'FD_CLOEXEC': '1',
   'FD_SETSIZE': '64',
   'FEXCL': '2048',
   'FEXLOCK': '256',
   'FIONREAD': '1',
   'FLT_EVAL_METHOD': '0',
   'FMARK': '16',
   'FNBIO': '4096',
   'FNDELAY': '16384',
   'FNOCTTY': '32768',
   'FNONBIO': '16384',
   'FOPEN': '-1',
   'FP_ILOGBNAN': '2147483647',
   'FP_INFINITE': '1',
   'FP_NAN': '0',
   'FP_NORMAL': '4',
   'FP_SUBNORMAL': '3',
   'FP_ZERO': '2',
   'FREAD': '1',
   'FSHLOCK': '128',
   'FSYNC': '8192',
   'FTRUNC': '1024',
   'FWRITE': '2',
   'F_CNVT': '12',
   'F_DUPFD': '0',
   'F_DUPFD_CLOEXEC': '14',
   'F_GETFD': '1',
   'F_GETFL': '3',
   'F_GETLK': '7',
   'F_GETLK64': '20',
   'F_GETOWN': '5',
   'F_LOCK': '1',
   'F_OK': '0',
   'F_RDLCK': '1',
   'F_RGETLK': '10',
   'F_RSETLK': '11',
   'F_RSETLKW': '13',
   'F_SETFD': '2',
   'F_SETFL': '4',
   'F_SETLK': '8',
   'F_SETLK64': '21',
   'F_SETLKW': '9',
   'F_SETLKW64': '22',
   'F_SETOWN': '6',
   'F_TEST': '3',
   'F_TLOCK': '2',
   'F_ULOCK': '0',
   'F_UNLCK': '3',
   'F_UNLKSYS': '4',
   'F_WRLCK': '2',
   'H8300': '1',
   'HAVE_ABS': '1',
   'HAVE_ALLOCA': '1',
   'HAVE_ALLOCA_H': '1',
   'HAVE_ATAN': '1',
   'HAVE_ATAN2': '1',
   'HAVE_ATOF': '1',
   'HAVE_ATOI': '1',
   'HAVE_BCOPY': '1',
   'HAVE_CALLOC': '1',
   'HAVE_CEIL': '1',
   'HAVE_COPYSIGN': '1',
   'HAVE_COS': '1',
   'HAVE_COSF': '1',
   'HAVE_CTYPE_H': '1',
   'HAVE_FABS': '1',
   'HAVE_FLOOR': '1',
   'HAVE_FREE': '1',
   'HAVE_GCC_ATOMICS': '1',
   'HAVE_GCC_SYNC_LOCK_TEST_AND_SET': '1',
   'HAVE_GETENV': '1',
   'HAVE_INDEX': '1',
   'HAVE_INTTYPES_H': '1',
   'HAVE_ITOA': '1',
   'HAVE_LOG': '1',
   'HAVE_MALLOC': '1',
   'HAVE_MATH_H': '1',
   'HAVE_MEMCMP': '1',
   'HAVE_MEMCPY': '1',
   'HAVE_MEMMOVE': '1',
   'HAVE_MEMSET': '1',
   'HAVE_M_PI': '1',
   'HAVE_NANOSLEEP': '1',
   'HAVE_POW': '1',
   'HAVE_PUTENV': '1',
   'HAVE_QSORT': '1',
   'HAVE_REALLOC': '1',
   'HAVE_RINDEX': '1',
   'HAVE_SCALBN': '1',
   'HAVE_SETENV': '1',
   'HAVE_SETJMP': '1',
   'HAVE_SIGACTION': '1',
   'HAVE_SIGNAL_H': '1',
   'HAVE_SIN': '1',
   'HAVE_SINF': '1',
   'HAVE_SNPRINTF': '1',
   'HAVE_SQRT': '1',
   'HAVE_SSCANF': '1',
   'HAVE_STDARG_H': '1',
   'HAVE_STDDEF_H': '1',
   'HAVE_STDINT_H': '1',
   'HAVE_STDIO_H': '1',
   'HAVE_STRCASECMP': '1',
   'HAVE_STRCHR': '1',
   'HAVE_STRCMP': '1',
   'HAVE_STRDUP': '1',
   'HAVE_STRICMP': '1',
   'HAVE_STRING_H': '1',
   'HAVE_STRLCAT': '1',
   'HAVE_STRLCPY': '1',
   'HAVE_STRLEN': '1',
   'HAVE_STRNCASECMP': '1',
   'HAVE_STRNCMP': '1',
   'HAVE_STRRCHR': '1',
   'HAVE_STRSTR': '1',
   'HAVE_STRTOD': '1',
   'HAVE_STRTOL': '1',
   'HAVE_STRTOLL': '1',
   'HAVE_STRTOUL': '1',
   'HAVE_STRTOULL': '1',
   'HAVE_SYSCONF': '1',
   'HAVE_SYSCTLBYNAME': '1',
   'HAVE_SYS_TYPES_H': '1',
   'HAVE_UNSETENV': '1',
   'HAVE_VSNPRINTF': '1',
   'HAVE__LTOA': '1',
   'HAVE__STRICMP': '1',
   'HAVE__STRLWR': '1',
   'HAVE__STRNICMP': '1',
   'HAVE__STRREV': '1',
   'HAVE__STRUPR': '1',
   'HAVE__ULTOA': '1',
   'HUGE_VAL': 'inf',
   'INT_MAX': '2147483647',
   'IPPROTO_TCP': '6',
   'IPPROTO_UDP': '17',
   'ITIMER_PROF': '2',
   'ITIMER_REAL': '0',
   'ITIMER_VIRTUAL': '1',
   'LACKS_SYS_MMAN_H': '1',
   'LONG_MAX': '2147483647',
   'MAC_OS_X_VERSION_10_4': '1040',
   'MAC_OS_X_VERSION_10_5': '1050',
   'MAC_OS_X_VERSION_10_6': '1060',
   'MALLOC_ALIGNMENT': '16',
   'MATH_ERREXCEPT': '2',
   'math_errhandling': '1',
   'MATH_ERRNO': '1',
   'MAXPATHLEN': '1024',
   'MB_LEN_MAX': '1',
   'MON_1': '21',
   'MON_10': '30',
   'MON_11': '31',
   'MON_12': '32',
   'MON_2': '22',
   'MON_3': '23',
   'MON_4': '24',
   'MON_5': '25',
   'MON_6': '26',
   'MON_7': '27',
   'MON_8': '28',
   'MON_9': '29',
   'M_1_PI': '0.318309886184',
   'M_2_PI': '0.636619772368',
   'M_2_SQRTPI': '1.1283791671',
   'M_3PI_4': '2.35619449019',
   'M_E': '2.71828182846',
   'M_INVLN2': '1.44269504089',
   'M_IVLN10': '0.434294481903',
   'M_LN10': '2.30258509299',
   'M_LN2': '0.69314718056',
   'M_LN2HI': '0.693147180369',
   'M_LN2LO': '1.90821492927e-10',
   'M_LOG10E': '0.434294481903',
   'M_LOG2E': '1.44269504089',
   'M_LOG2_E': '0.69314718056',
   'M_PI': '3.14159265359',
   'M_PI_2': '1.57079632679',
   'M_PI_4': '0.785398163397',
   'M_SQRT1_2': '0.707106781187',
   'M_SQRT2': '1.41421356237',
   'M_SQRT3': '1.73205080757',
   'M_SQRTPI': '1.77245385091',
   'M_TWOPI': '6.28318530718',
   'NBBY': '8',
   'NL_ARGMAX': '32',
   'NOEXPR': '53',
   'NOSTR': '55',
   'NO_ARG': '0',
   'no_argument': '0',
   'NSIG': '64',
   'NULL': '0',
   'OPTIONAL_ARG': '2',
   'optional_argument': '2',
   'OPT_ARG': '2',
   'OVERFLOW': '3',
   'O_ACCMODE': '3',
   'O_APPEND': '8',
   'O_BINARY': '65536',
   'O_CLOEXEC': '262144',
   'O_CREAT': '512',
   'O_EXCL': '2048',
   'O_NOCTTY': '32768',
   'O_NOINHERIT': '262144',
   'O_NONBLOCK': '16384',
   'O_RDONLY': '0',
   'O_RDWR': '2',
   'O_SYNC': '8192',
   'O_TEXT': '131072',
   'O_TRUNC': '1024',
   'O_WRONLY': '1',
   'PATH_MAX': '4096',
   'PF_INET': '2',
   'PF_INET6': '6',
   'PLOSS': '6',
   'PM_STR': '6',
   'POLLERR': '8',
   'POLLHUP': '16',
   'POLLPRI': '32',
   'POLLIN': '1',
   'POLLNVAL': '4',
   'POLLOUT': '2',
   'POSIX_FADV_DONTNEED': '135',
   'PTHREAD_CREATE_DETACHED': '0',
   'PTHREAD_CREATE_JOINABLE': '1',
   'PTHREAD_EXPLICIT_SCHED': '2',
   'PTHREAD_INHERIT_SCHED': '1',
   'PTHREAD_MUTEX_DEFAULT': '3',
   'PTHREAD_MUTEX_ERRORCHECK': '2',
   'PTHREAD_MUTEX_NORMAL': '0',
   'PTHREAD_MUTEX_RECURSIVE': '1',
   'PTHREAD_PRIO_INHERIT': '1',
   'PTHREAD_PRIO_NONE': '0',
   'PTHREAD_PRIO_PROTECT': '2',
   'PTHREAD_PROCESS_PRIVATE': '0',
   'PTHREAD_PROCESS_SHARED': '1',
   'PTHREAD_SCOPE_PROCESS': '0',
   'PTHREAD_SCOPE_SYSTEM': '1',
   'PTHREAD_STACK_MIN': '200',
   'RADIXCHAR': '50',
   'REQUIRED_ARG': '1',
   'required_argument': '1',
   'REQ_ARG': '1',
   'R_OK': '4',
   'SA_NOCLDSTOP': '1',
   'SA_NODEFER': '4',
   'SA_RESETHAND': '8',
   'SA_SIGINFO': '2',
   'SCHAR_MAX': '127',
   'SCHAR_MIN': '-128',
   'SCHED_FIFO': '1',
   'SCHED_OTHER': '0',
   'SCHED_RR': '2',
   'SCHED_SPORADIC': '4',
   'SDL_ALTIVEC_BLITTERS': '1',
   'SDL_ASSEMBLY_ROUTINES': '1',
   'SDL_ASSERT_LEVEL': '1',
   'SDL_AUDIO_DRIVER_ANDROID': '1',
   'SDL_AUDIO_DRIVER_COREAUDIO': '1',
   'SDL_AUDIO_DRIVER_COREAUDIOIPHONE': '1',
   'SDL_AUDIO_DRIVER_DISK': '1',
   'SDL_AUDIO_DRIVER_DSOUND': '1',
   'SDL_AUDIO_DRIVER_DUMMY': '1',
   'SDL_AUDIO_DRIVER_NDS': '1',
   'SDL_AUDIO_DRIVER_WINMM': '1',
   'SDL_AUDIO_DRIVER_XAUDIO2': '1',
   'SDL_HAPTIC_DINPUT': '1',
   'SDL_HAPTIC_DISABLED': '1',
   'SDL_HAPTIC_DUMMY': '1',
   'SDL_HAPTIC_IOKIT': '1',
   'SDL_HAPTIC_NDS': '1',
   'SDL_IPHONE_KEYBOARD': '1',
   'SDL_IPHONE_MAX_GFORCE': '5.0',
   'SDL_JOYSTICK_ANDROID': '1',
   'SDL_JOYSTICK_DINPUT': '1',
   'SDL_JOYSTICK_DISABLED': '1',
   'SDL_JOYSTICK_IOKIT': '1',
   'SDL_JOYSTICK_NDS': '1',
   'SDL_LOADSO_DISABLED': '1',
   'SDL_LOADSO_DLOPEN': '1',
   'SDL_LOADSO_WINDOWS': '1',
   'SDL_POWER_MACOSX': '1',
   'SDL_POWER_NINTENDODS': '1',
   'SDL_POWER_UIKIT': '1',
   'SDL_POWER_WINDOWS': '1',
   'SDL_THREADS_DISABLED': '1',
   'SDL_THREAD_PTHREAD': '1',
   'SDL_THREAD_PTHREAD_RECURSIVE_MUTEX': '1',
   'SDL_THREAD_WINDOWS': '1',
   'SDL_TIMERS_DISABLED': '1',
   'SDL_TIMER_NDS': '1',
   'SDL_TIMER_UNIX': '1',
   'SDL_TIMER_WINCE': '1',
   'SDL_TIMER_WINDOWS': '1',
   'SDL_VIDEO_DRIVER_ANDROID': '1',
   'SDL_VIDEO_DRIVER_COCOA': '1',
   'SDL_VIDEO_DRIVER_DUMMY': '1',
   'SDL_VIDEO_DRIVER_NDS': '1',
   'SDL_VIDEO_DRIVER_UIKIT': '1',
   'SDL_VIDEO_DRIVER_WINDOWS': '1',
   'SDL_VIDEO_DRIVER_X11': '1',
   'SDL_VIDEO_DRIVER_X11_XINERAMA': '1',
   'SDL_VIDEO_DRIVER_X11_XINPUT': '1',
   'SDL_VIDEO_DRIVER_X11_XRANDR': '1',
   'SDL_VIDEO_DRIVER_X11_XSCRNSAVER': '1',
   'SDL_VIDEO_DRIVER_X11_XSHAPE': '1',
   'SDL_VIDEO_DRIVER_X11_XVIDMODE': '1',
   'SDL_VIDEO_OPENGL': '1',
   'SDL_VIDEO_OPENGL_CGL': '1',
   'SDL_VIDEO_OPENGL_ES': '1',
   'SDL_VIDEO_OPENGL_GLX': '1',
   'SDL_VIDEO_OPENGL_WGL': '1',
   'SDL_VIDEO_RENDER_D3D': '1',
   'SDL_VIDEO_RENDER_NDS': '0',
   'SDL_VIDEO_RENDER_OGL': '1',
   'SDL_VIDEO_RENDER_OGL_ES': '1',
   'SDL_VIDEO_RENDER_OGL_ES2': '1',
   'SEEK_CUR': '1',
   'SEEK_END': '2',
   'SEEK_SET': '0',
   'SHRT_MAX': '32767',
   'SHRT_MIN': '-32768',
   'SIGABRT': '6',
   'SIGALRM': '14',
   'SIGBUS': '10',
   'SIGCHLD': '20',
   'SIGCLD': '20',
   'SIGCONT': '19',
   'SIGEMT': '7',
   'SIGEV_NONE': '1',
   'SIGEV_SIGNAL': '2',
   'SIGEV_THREAD': '3',
   'SIGFPE': '8',
   'SIGHUP': '1',
   'SIGILL': '4',
   'SIGINT': '2',
   'SIGIO': '23',
   'SIGIOT': '6',
   'SIGKILL': '9',
   'SIGLOST': '29',
   'SIGPIPE': '13',
   'SIGPOLL': '23',
   'SIGPROF': '27',
   'SIGPWR': '19',
   'SIGQUIT': '3',
   'SIGRTMAX': '64',
   'SIGRTMIN': '32',
   'SIGSEGV': '11',
   'SIGSTOP': '17',
   'SIGSYS': '12',
   'SIGTERM': '15',
   'SIGTRAP': '5',
   'SIGTSTP': '18',
   'SIGTTIN': '21',
   'SIGTTOU': '22',
   'SIGURG': '16',
   'SIGUSR1': '30',
   'SIGUSR2': '31',
   'SIGVTALRM': '26',
   'SIGWINCH': '28',
   'SIGXCPU': '24',
   'SIGXFSZ': '25',
   'SIG_BLOCK': '1',
   'SIG_SETMASK': '0',
   'SIG_UNBLOCK': '2',
   'SING': '2',
   'SIZEOF_VOIDP': '4',
   'SI_ASYNCIO': '4',
   'SI_MESGQ': '5',
   'SI_QUEUE': '2',
   'SI_TIMER': '3',
   'SI_USER': '1',
   'SOCK_DGRAM': '2',
   'SOCK_STREAM': '1',
   'STDC_HEADERS': '1',
   'STDERR_FILENO': '3',
   'STDIN_FILENO': '1',
   'STDOUT_FILENO': '2',
   'S_BLKSIZE': '1024',
   'S_ENFMT': '0002000',
   'S_IEXEC': '0000100',
   'S_IFBLK': '0060000',
   'S_IFCHR': '0020000',
   'S_IFDIR': '0040000',
   'S_IFIFO': '0010000',
   'S_IFLNK': '0120000',
   'S_IFMT': '0170000',
   'S_IFREG': '0100000',
   'S_IFSOCK': '0140000',
   'S_IREAD': '0000400',
   'S_IRGRP': '0000040',
   'S_IROTH': '0000004',
   'S_IRUSR': '0000400',
   'S_IRWXG': '0000070',
   'S_IRWXO': '0000007',
   'S_IRWXU': '0000700',
   'S_ISGID': '0002000',
   'S_ISUID': '0004000',
   'S_ISVTX': '0001000',
   'S_IWGRP': '0000020',
   'S_IWOTH': '0000002',
   'S_IWRITE': '0000200',
   'S_IWUSR': '0000200',
   'S_IXGRP': '0000010',
   'S_IXOTH': '0000001',
   'S_IXUSR': '0000100',
   'THOUSEP': '51',
   'TIMER_ABSTIME': '4',
   'TLOSS': '5',
   'T_FMT': '3',
   'T_FMT_AMPM': '4',
   'UCHAR_MAX': '255',
   'UINT_MAX': '2147483647',
   'ULONG_MAX': '2147483647',
   'UNDERFLOW': '4',
   'USHRT_MAX': '65535',
   'UTIME_NOW': '-2',
   'UTIME_OMIT': '-1',
   'W_OK': '2',
   'X_OK': '1',
   'YESEXPR': '52',
   'YESSTR': '54',
   '_ATEXIT_SIZE': '32',
   '_CLOCKS_PER_SEC_': '1000',
   '_CS_GNU_LIBC_VERSION': '42',
   '_CS_GNU_LIBPTHREAD_VERSION': '43',
   '_CS_PATH': '0',
   '_CS_POSIX_V6_ILP32_OFF32_CFLAGS': '1',
   '_CS_POSIX_V6_ILP32_OFF32_LDFLAGS': '2',
   '_CS_POSIX_V6_ILP32_OFF32_LIBS': '3',
   '_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS': '5',
   '_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS': '6',
   '_CS_POSIX_V6_ILP32_OFFBIG_LIBS': '7',
   '_CS_POSIX_V6_LP64_OFF64_CFLAGS': '9',
   '_CS_POSIX_V6_LP64_OFF64_LDFLAGS': '10',
   '_CS_POSIX_V6_LP64_OFF64_LIBS': '11',
   '_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS': '13',
   '_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS': '14',
   '_CS_POSIX_V6_LPBIG_OFFBIG_LIBS': '15',
   '_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS': '17',
   '_CS_POSIX_V7_ILP32_OFF32_CFLAGS': '1',
   '_CS_POSIX_V7_ILP32_OFF32_LDFLAGS': '2',
   '_CS_POSIX_V7_ILP32_OFF32_LIBS': '3',
   '_CS_POSIX_V7_ILP32_OFFBIG_CFLAGS': '5',
   '_CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS': '6',
   '_CS_POSIX_V7_ILP32_OFFBIG_LIBS': '7',
   '_CS_POSIX_V7_LP64_OFF64_CFLAGS': '9',
   '_CS_POSIX_V7_LP64_OFF64_LDFLAGS': '10',
   '_CS_POSIX_V7_LP64_OFF64_LIBS': '11',
   '_CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS': '13',
   '_CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS': '14',
   '_CS_POSIX_V7_LPBIG_OFFBIG_LIBS': '15',
   '_CS_POSIX_V7_THREADS_CFLAGS': '18',
   '_CS_POSIX_V7_THREADS_LDFLAGS': '19',
   '_CS_POSIX_V7_WIDTH_RESTRICTED_ENVS': '17',
   '_CS_V7_ENV': '20',
   '_CS_XBS5_ILP32_OFF32_CFLAGS': '1',
   '_CS_XBS5_ILP32_OFF32_LDFLAGS': '2',
   '_CS_XBS5_ILP32_OFF32_LIBS': '3',
   '_CS_XBS5_ILP32_OFF32_LINTFLAGS': '4',
   '_CS_XBS5_ILP32_OFFBIG_CFLAGS': '5',
   '_CS_XBS5_ILP32_OFFBIG_LDFLAGS': '6',
   '_CS_XBS5_ILP32_OFFBIG_LIBS': '7',
   '_CS_XBS5_ILP32_OFFBIG_LINTFLAGS': '8',
   '_CS_XBS5_LP64_OFF64_CFLAGS': '9',
   '_CS_XBS5_LP64_OFF64_LDFLAGS': '10',
   '_CS_XBS5_LP64_OFF64_LIBS': '11',
   '_CS_XBS5_LP64_OFF64_LINTFLAGS': '12',
   '_CS_XBS5_LPBIG_OFFBIG_CFLAGS': '13',
   '_CS_XBS5_LPBIG_OFFBIG_LDFLAGS': '14',
   '_CS_XBS5_LPBIG_OFFBIG_LIBS': '15',
   '_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS': '16',
   '_DATE_FMT': '84',
   '_FAPPEND': '8',
   '_FASYNC': '64',
   '_FBINARY': '65536',
   '_FCREAT': '512',
   '_FDEFER': '32',
   '_FEXCL': '2048',
   '_FEXLOCK': '256',
   '_FLOAT_ARG': "<type 'float'>",
   '_FMARK': '16',
   '_FNBIO': '4096',
   '_FNDELAY': '16384',
   '_FNOCTTY': '32768',
   '_FNOINHERIT': '262144',
   '_FNONBLOCK': '16384',
   '_FOPEN': '-1',
   '_FREAD': '1',
   '_FSHLOCK': '128',
   '_FSYNC': '8192',
   '_FTEXT': '131072',
   '_FTRUNC': '1024',
   '_FWRITE': '2',
   '_IFBLK': '0060000',
   '_IFCHR': '0020000',
   '_IFDIR': '0040000',
   '_IFIFO': '0010000',
   '_IFLNK': '0120000',
   '_IFMT': '0170000',
   '_IFREG': '0100000',
   '_IFSOCK': '0140000',
   '_LARGEFILE64_SOURCE': '1',
   '_LIBC_LIMITS_H_': '1',
   '_LIMITS_H': '1',
   '_LONG_LONG_TYPE': "<type 'long'>",
   '_MB_EXTENDED_CHARSETS_ISO': '1',
   '_MB_EXTENDED_CHARSETS_WINDOWS': '1',
   '_M_LN2': '0.69314718056',
   '_NL_CTYPE_CODESET_NAME': '0',
   '_NL_CTYPE_MB_CUR_MAX': '85',
   '_NL_MESSAGES_CODESET': '86',
   '_NL_TIME_DATE_FMT': '84',
   '_NULL': '0',
   '_N_LISTS': '30',
   '_O_APPEND': '8',
   '_O_BINARY': '65536',
   '_O_CREAT': '512',
   '_O_EXCL': '2048',
   '_O_NOINHERIT': '262144',
   '_O_RAW': '65536',
   '_O_RDONLY': '0',
   '_O_RDWR': '2',
   '_O_TEXT': '131072',
   '_O_TRUNC': '1024',
   '_O_WRONLY': '1',
   '_PC_2_SYMLINKS': '13',
   '_PC_ALLOC_SIZE_MIN': '15',
   '_PC_ASYNC_IO': '9',
   '_PC_CHOWN_RESTRICTED': '6',
   '_PC_FILESIZEBITS': '12',
   '_PC_LINK_MAX': '0',
   '_PC_MAX_CANON': '1',
   '_PC_MAX_INPUT': '2',
   '_PC_NAME_MAX': '3',
   '_PC_NO_TRUNC': '7',
   '_PC_PATH_MAX': '4',
   '_PC_PIPE_BUF': '5',
   '_PC_POSIX_PERMISSIONS': '90',
   '_PC_POSIX_SECURITY': '91',
   '_PC_PRIO_IO': '10',
   '_PC_REC_INCR_XFER_SIZE': '16',
   '_PC_REC_MAX_XFER_SIZE': '17',
   '_PC_REC_MIN_XFER_SIZE': '18',
   '_PC_REC_XFER_ALIGN': '19',
   '_PC_SOCK_MAXBUF': '100',
   '_PC_SYMLINK_MAX': '14',
   '_PC_SYNC_IO': '11',
   '_PC_TIMESTAMP_RESOLUTION': '20',
   '_PC_VDISABLE': '8',
   '_POINTER_INT': "<type 'long'>",
   '_POSIX2_CHAR_TERM': '200112',
   '_POSIX2_C_BIND': '200112',
   '_POSIX2_C_DEV': '200112',
   '_POSIX2_RE_DUP_MAX': '255',
   '_POSIX2_SW_DEV': '200112',
   '_POSIX2_UPE': '200112',
   '_POSIX2_VERSION': '200112',
   '_POSIX_ADVISORY_INFO': '200112',
   '_POSIX_ASYNCHRONOUS_IO': '1',
   '_POSIX_BARRIERS': '200112',
   '_POSIX_CHOWN_RESTRICTED': '1',
   '_POSIX_CPUTIME': '1',
   '_POSIX_C_SOURCE': '2',
   '_POSIX_DEVCTL_DIRECTION': '1',
   '_POSIX_DEVICE_CONTROL': '1',
   '_POSIX_FSYNC': '200112',
   '_POSIX_INTERRUPT_CONTROL': '1',
   '_POSIX_IPV6': '200112',
   '_POSIX_JOB_CONTROL': '1',
   '_POSIX_MAPPED_FILES': '200112',
   '_POSIX_MEMLOCK': '1',
   '_POSIX_MEMLOCK_RANGE': '200112',
   '_POSIX_MEMORY_PROTECTION': '200112',
   '_POSIX_MESSAGE_PASSING': '200112',
   '_POSIX_MONOTONIC_CLOCK': '200112',
   '_POSIX_NO_TRUNC': '1',
   '_POSIX_PRIORITIZED_IO': '1',
   '_POSIX_PRIORITY_SCHEDULING': '200112',
   '_POSIX_RAW_SOCKETS': '200112',
   '_POSIX_READER_WRITER_LOCKS': '200112',
   '_POSIX_REALTIME_SIGNALS': '200112',
   '_POSIX_REGEXP': '1',
   '_POSIX_SAVED_IDS': '1',
   '_POSIX_SEMAPHORES': '200112',
   '_POSIX_SHARED_MEMORY_OBJECTS': '200112',
   '_POSIX_SHELL': '1',
   '_POSIX_SPAWN': '1',
   '_POSIX_SPIN_LOCKS': '200112',
   '_POSIX_SPORADIC_SERVER': '1',
   '_POSIX_SYNCHRONIZED_IO': '200112',
   '_POSIX_THREADS': '200112',
   '_POSIX_THREAD_ATTR_STACKADDR': '1',
   '_POSIX_THREAD_ATTR_STACKSIZE': '200112',
   '_POSIX_THREAD_CPUTIME': '1',
   '_POSIX_THREAD_PRIORITY_SCHEDULING': '200112',
   '_POSIX_THREAD_PRIO_INHERIT': '1',
   '_POSIX_THREAD_PRIO_PROTECT': '1',
   '_POSIX_THREAD_PROCESS_SHARED': '200112',
   '_POSIX_THREAD_SAFE_FUNCTIONS': '200112',
   '_POSIX_THREAD_SPORADIC_SERVER': '1',
   '_POSIX_TIMEOUTS': '1',
   '_POSIX_TIMERS': '1',
   '_POSIX_V6_ILP32_OFF32': '-1',
   '_POSIX_V6_ILP32_OFFBIG': '1',
   '_POSIX_V6_LP64_OFF64': '-1',
   '_POSIX_V6_LPBIG_OFFBIG': '-1',
   '_POSIX_VERSION': '200112',
   '_RAND48_ADD': '11',
   '_RAND48_MULT_0': '58989',
   '_RAND48_MULT_1': '57068',
   '_RAND48_MULT_2': '5',
   '_RAND48_SEED_0': '13070',
   '_RAND48_SEED_1': '43981',
   '_RAND48_SEED_2': '4660',
   '_READ_WRITE_RETURN_TYPE': "<type 'int'>",
   '_REENT_ASCTIME_SIZE': '26',
   '_REENT_EMERGENCY_SIZE': '25',
   '_REENT_SIGNAL_SIZE': '24',
   '_SC_2_CHAR_TERM': '107',
   '_SC_2_C_BIND': '108',
   '_SC_2_C_DEV': '109',
   '_SC_2_FORT_DEV': '110',
   '_SC_2_FORT_RUN': '111',
   '_SC_2_LOCALEDEF': '112',
   '_SC_2_PBS': '113',
   '_SC_2_PBS_ACCOUNTING': '114',
   '_SC_2_PBS_CHECKPOINT': '115',
   '_SC_2_PBS_LOCATE': '116',
   '_SC_2_PBS_MESSAGE': '117',
   '_SC_2_PBS_TRACK': '118',
   '_SC_2_SW_DEV': '119',
   '_SC_2_UPE': '120',
   '_SC_2_VERSION': '121',
   '_SC_ADVISORY_INFO': '54',
   '_SC_AIO_LISTIO_MAX': '34',
   '_SC_AIO_MAX': '35',
   '_SC_AIO_PRIO_DELTA_MAX': '36',
   '_SC_ARG_MAX': '0',
   '_SC_ASYNCHRONOUS_IO': '21',
   '_SC_ATEXIT_MAX': '55',
   '_SC_AVPHYS_PAGES': '12',
   '_SC_BARRIERS': '56',
   '_SC_BC_BASE_MAX': '57',
   '_SC_BC_DIM_MAX': '58',
   '_SC_BC_SCALE_MAX': '59',
   '_SC_BC_STRING_MAX': '60',
   '_SC_CHILD_MAX': '1',
   '_SC_CLK_TCK': '2',
   '_SC_CLOCK_SELECTION': '61',
   '_SC_COLL_WEIGHTS_MAX': '62',
   '_SC_CPUTIME': '63',
   '_SC_DELAYTIMER_MAX': '37',
   '_SC_EXPR_NEST_MAX': '64',
   '_SC_FSYNC': '22',
   '_SC_GETGR_R_SIZE_MAX': '50',
   '_SC_GETPW_R_SIZE_MAX': '51',
   '_SC_HOST_NAME_MAX': '65',
   '_SC_IOV_MAX': '66',
   '_SC_IPV6': '67',
   '_SC_JOB_CONTROL': '5',
   '_SC_LINE_MAX': '68',
   '_SC_LOGIN_NAME_MAX': '52',
   '_SC_MAPPED_FILES': '23',
   '_SC_MEMLOCK': '24',
   '_SC_MEMLOCK_RANGE': '25',
   '_SC_MEMORY_PROTECTION': '26',
   '_SC_MESSAGE_PASSING': '27',
   '_SC_MONOTONIC_CLOCK': '69',
   '_SC_MQ_OPEN_MAX': '13',
   '_SC_MQ_PRIO_MAX': '14',
   '_SC_NGROUPS_MAX': '3',
   '_SC_NPROCESSORS_CONF': '9',
   '_SC_NPROCESSORS_ONLN': '10',
   '_SC_OPEN_MAX': '4',
   '_SC_PAGESIZE': '8',
   '_SC_PAGE_SIZE': '8',
   '_SC_PHYS_PAGES': '11',
   '_SC_PRIORITIZED_IO': '28',
   '_SC_PRIORITY_SCHEDULING': '101',
   '_SC_RAW_SOCKETS': '70',
   '_SC_READER_WRITER_LOCKS': '71',
   '_SC_REALTIME_SIGNALS': '29',
   '_SC_REGEXP': '72',
   '_SC_RE_DUP_MAX': '73',
   '_SC_RTSIG_MAX': '15',
   '_SC_SAVED_IDS': '6',
   '_SC_SEMAPHORES': '30',
   '_SC_SEM_NSEMS_MAX': '16',
   '_SC_SEM_VALUE_MAX': '17',
   '_SC_SHARED_MEMORY_OBJECTS': '199',
   '_SC_SHELL': '74',
   '_SC_SIGQUEUE_MAX': '18',
   '_SC_SPAWN': '75',
   '_SC_SPIN_LOCKS': '76',
   '_SC_SPORADIC_SERVER': '77',
   '_SC_SS_REPL_MAX': '78',
   '_SC_STREAM_MAX': '100',
   '_SC_SYMLOOP_MAX': '79',
   '_SC_SYNCHRONIZED_IO': '32',
   '_SC_THREADS': '42',
   '_SC_THREAD_ATTR_STACKADDR': '43',
   '_SC_THREAD_ATTR_STACKSIZE': '44',
   '_SC_THREAD_CPUTIME': '80',
   '_SC_THREAD_DESTRUCTOR_ITERATIONS': '53',
   '_SC_THREAD_KEYS_MAX': '38',
   '_SC_THREAD_PRIORITY_SCHEDULING': '45',
   '_SC_THREAD_PRIO_CEILING': '47',
   '_SC_THREAD_PRIO_INHERIT': '46',
   '_SC_THREAD_PRIO_PROTECT': '47',
   '_SC_THREAD_PROCESS_SHARED': '48',
   '_SC_THREAD_ROBUST_PRIO_INHERIT': '122',
   '_SC_THREAD_ROBUST_PRIO_PROTECT': '123',
   '_SC_THREAD_SAFE_FUNCTIONS': '49',
   '_SC_THREAD_SPORADIC_SERVER': '81',
   '_SC_THREAD_STACK_MIN': '39',
   '_SC_THREAD_THREADS_MAX': '40',
   '_SC_TIMEOUTS': '82',
   '_SC_TIMERS': '33',
   '_SC_TIMER_MAX': '19',
   '_SC_TRACE': '83',
   '_SC_TRACE_EVENT_FILTER': '84',
   '_SC_TRACE_EVENT_NAME_MAX': '85',
   '_SC_TRACE_INHERIT': '86',
   '_SC_TRACE_LOG': '87',
   '_SC_TRACE_NAME_MAX': '88',
   '_SC_TRACE_SYS_MAX': '89',
   '_SC_TRACE_USER_EVENT_MAX': '90',
   '_SC_TTY_NAME_MAX': '41',
   '_SC_TYPED_MEMORY_OBJECTS': '91',
   '_SC_TZNAME_MAX': '20',
   '_SC_V6_ILP32_OFF32': '92',
   '_SC_V6_ILP32_OFFBIG': '93',
   '_SC_V6_LP64_OFF64': '94',
   '_SC_V6_LPBIG_OFFBIG': '95',
   '_SC_V7_ILP32_OFF32': '92',
   '_SC_V7_ILP32_OFFBIG': '93',
   '_SC_V7_LP64_OFF64': '94',
   '_SC_V7_LPBIG_OFFBIG': '95',
   '_SC_VERSION': '7',
   '_SC_XBS5_ILP32_OFF32': '92',
   '_SC_XBS5_ILP32_OFFBIG': '93',
   '_SC_XBS5_LP64_OFF64': '94',
   '_SC_XBS5_LPBIG_OFFBIG': '95',
   '_SC_XOPEN_CRYPT': '96',
   '_SC_XOPEN_ENH_I18N': '97',
   '_SC_XOPEN_LEGACY': '98',
   '_SC_XOPEN_REALTIME': '99',
   '_SC_XOPEN_REALTIME_THREADS': '102',
   '_SC_XOPEN_SHM': '103',
   '_SC_XOPEN_STREAMS': '104',
   '_SC_XOPEN_UNIX': '105',
   '_SC_XOPEN_UUCP': '124',
   '_SC_XOPEN_VERSION': '106',
   '_S_IEXEC': '0000100',
   '_S_IFCHR': '0020000',
   '_S_IFDIR': '0040000',
   '_S_IFIFO': '0010000',
   '_S_IFMT': '0170000',
   '_S_IFREG': '0100000',
   '_S_IREAD': '0000400',
   '_S_IWRITE': '0000200',
   '_TIME_T_': "<type 'long'>",
   '_UNIX98_THREAD_MUTEX_ATTRIBUTES': '1',
   '_XBS5_ILP32_OFF32': '-1',
   '_XBS5_ILP32_OFFBIG': '1',
   '_XBS5_LP64_OFF64': '-1',
   '_XBS5_LPBIG_OFFBIG': '-1',
   '_XOPEN_CRYPT': '1',
   '_XOPEN_ENH_I18N': '1',
   '_XOPEN_SHM': '1',
   '_XOPEN_VERSION': '600',
   '__AIX__': '1',
   '__ANDROID__': '1',
   '__BEOS__': '1',
   '__BSDI__': '1',
   '__BUFSIZ__': '16',
   '__DREAMCAST__': '1',
   '__FILENAME_MAX__': '255',
   '__FREEBSD__': '1',
   '__GNUC_VA_LIST': '1',
   '__HAIKU__': '1',
   '__HPUX__': '1',
   '__INT_MAX__': '2147483647',
   '__IPHONEOS__': '1',
   '__IRIX__': '1',
   '__LARGE64_FILES': '1',
   '__LINUX__': '1',
   '__long_double_t': "<type 'long'>",
   '__LONG_MAX__': '2147483647',
   '__MACOSX__': '1',
   '__NETBSD__': '1',
   '__NINTENDODS__': '1',
   '__OPENBSD__': '1',
   '__OS2__': '1',
   '__OSF__': '1',
   '__QNXNTO__': '1',
   '__RAND_MAX': '2147483647',
   '__RISCOS__': '1',
   '__SIGFIRSTNOTRT': '1',
   '__SIGLASTNOTRT': '31',
   '__SOLARIS__': '1',
   '__USE_XOPEN2K': '1',
   '__WIN32__': '1',
   '___int16_t_defined': '1',
   '___int32_t_defined': '1',
   '___int64_t_defined': '1',
   '___int8_t_defined': '1',
   '___int_least16_t_defined': '1',
   '___int_least32_t_defined': '1',
   '___int_least8_t_defined': '1',
    'FMODE_READ': '0x1',
   'FMODE_WRITE': '0x2',
   'FMODE_LSEEK': '0x4',
   'FMODE_PREAD': '0x8',
   'FMODE_PWRITE': '0x10',
   'FMODE_EXEC': '0x20',
   'FMODE_NDELAY': '0x40',
   'FMODE_EXCL': '0x80',
   'FMODE_NOCMTIME': '0x800',
   'FMODE_RANDOM': '0x1000',
   'FMODE_UNSIGNED_OFFSET': '0x2000',
   'FMODE_PATH': '0x4000',
   'FMODE_NONOTIFY': '0x1000000',
   'S_IRWXUGO': '511',
   'S_IALLUGO': '4095',
   'S_IRUGO': '292',
   'S_IWUGO': '146',
   'S_IXUGO': '73',
   'LOOKUP_FOLLOW': '0x0001',
   'LOOKUP_DIRECTORY': '0x0002',
   'LOOKUP_PARENT': '0x0010',
   'MAP_SHARED': '0x01',
   'MAP_PRIVATE': '0x02',
   'MAP_TYPE': '0x0f',
   'MAP_FIXED': '0x100',
   'MAP_ANONYMOUS': '0x10',
   'O_NOFOLLOW': '0200000',
   'EPERM': '1',
   'ENOENT': '2',
   'ESRCH': '3',
   'EINTR': '4',
   'EIO': '5',
   'ENXIO': '6',
   'E2BIG': '7',
   'ENOEXEC': '8',
   'EBADF': '9',
   'ECHILD': '10',
   'EAGAIN': '11',
   'EWOULDBLOCK': '11',
   'ENOMEM': '12',
   'EACCES': '13',
   'EFAULT': '14',
   'ENOTBLK': '15',
   'EBUSY': '16',
   'EEXIST': '17',
   'EXDEV': '18',
   'ENODEV': '19',
   'ENOTDIR': '20',
   'EISDIR': '21',
   'EINVAL': '22',
   'ENFILE': '23',
   'EMFILE': '24',
   'ENOTTY': '25',
   'ETXTBSY': '26',
   'EFBIG': '27',
   'ENOSPC': '28',
   'ESPIPE': '29',
   'EROFS': '30',
   'EMLINK': '31',
   'EPIPE': '32',
   'EDOM': '33',
   'ERANGE': '34',
   'ENOMSG': '35',
   'EIDRM': '36',
   'ECHRNG': '37',
   'EL2NSYNC': '38',
   'EL3HLT': '39',
   'EL3RST': '40',
   'ELNRNG': '41',
   'EUNATCH': '42',
   'ENOCSI': '43',
   'EL2HLT': '44',
   'EDEADLK': '45',
   'ENOLCK': '46',
   'EBADE': '50',
   'EBADR': '51',
   'EXFULL': '52',
   'ENOANO': '53',
   'EBADRQC': '54',
   'EBADSLT': '55',
   'EDEADLOCK': '56',
   'EBFONT': '57',
   'ENOSTR': '60',
   'ENODATA': '61',
   'ETIME': '62',
   'ENOSR': '63',
   'ENONET': '64',
   'ENOPKG': '65',
   'EREMOTE': '66',
   'ENOLINK': '67',
   'EADV': '68',
   'ESRMNT': '69',
   'ECOMM': '70',
   'EPROTO': '71',
   'EMULTIHOP': '74',
   'EDOTDOT': '76',
   'EBADMSG': '77',
   'ENOTUNIQ': '80',
   'EBADFD': '81',
   'EREMCHG': '82',
   'ELIBACC': '83',
   'ELIBBAD': '84',
   'ELIBSCN': '85',
   'ELIBMAX': '86',
   'ELIBEXEC': '87',
   'ENOSYS': '88',
   'ENOTEMPTY': '90',
   'ENAMETOOLONG': '91',
   'ELOOP': '92',
   'EOPNOTSUPP': '95',
   'EPFNOSUPPORT': '96',
   'ECONNRESET': '104',
   'ENOBUFS': '105',
   'EAFNOSUPPORT': '106',
   'EPROTOTYPE': '107',
   'ENOTSOCK': '108',
   'ENOPROTOOPT': '109',
   'ESHUTDOWN': '110',
   'ECONNREFUSED': '111',
   'EADDRINUSE': '112',
   'ECONNABORTED': '113',
   'ENETUNREACH': '114',
   'ENETDOWN': '115',
   'ETIMEDOUT': '116',
   'EHOSTDOWN': '117',
   'EHOSTUNREACH': '118',
   'EINPROGRESS': '119',
   'EALREADY': '120',
   'EDESTADDRREQ': '121',
   'EMSGSIZE': '122',
   'EPROTONOSUPPORT': '123',
   'ESOCKTNOSUPPORT': '124',
   'EADDRNOTAVAIL': '125',
   'ENETRESET': '126',
   'EISCONN': '127',
   'ENOTCONN': '128',
   'ETOOMANYREFS': '129',
   'EUSERS': '131',
   'EDQUOT': '132',
   'ESTALE': '133',
   'ENOTSUP': '134',
   'ENOMEDIUM': '135',
   'EILSEQ': '138',
   'EOVERFLOW': '139',
   'ECANCELED': '140',
   'ENOTRECOVERABLE': '141',
   'EOWNERDEAD': '142',
   'ESTRPIPE': '143',
   'AI_PASSIVE': '0x0001',
   'AI_CANONNAME': '0x0002',
   'AI_NUMERICHOST': '0x0004',
   'AI_V4MAPPED': '0x0008',
   'AI_ALL': '0x0010',
   'AI_ADDRCONFIG': '0x0020',
   'AI_NUMERICSERV': '0x0400',
   'EAI_ADDRFAMILY': '1',
   'EAI_AGAIN': '2',
   'EAI_BADFLAGS': '3',
   'EAI_FAIL': '4',
   'EAI_FAMILY': '5',
   'EAI_MEMORY': '6',
   'EAI_NODATA': '7',
   'EAI_NONAME': '8',
   'EAI_SERVICE': '9',
   'EAI_SOCKTYPE': '10',
   'EAI_SYSTEM': '11',
   'EAI_BADHINTS': '12',
   'EAI_PROTOCOL': '13',
   'EAI_OVERFLOW': '14',
   'EAI_MAX': '15',
   'NI_NOFQDN': '0x00000001',
   'NI_NUMERICHOST': '0x00000002',
   'NI_NAMEREQD': '0x00000004',
   'NI_NUMERICSERV': '0x00000008',
   'NI_DGRAM': '0x00000010',
   'INADDR_ANY': '0',
   'INADDR_LOOPBACK': '0x7f000001'
};

