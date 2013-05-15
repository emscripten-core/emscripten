
//
// Various compiling-to-JS parameters. These are simply variables present when the
// JS compiler runs. To set them, do something like
//
//   emcc -s OPTION1=VALUE1 -s OPTION2=VALUE2 [..other stuff..]
//
// See https://github.com/kripken/emscripten/wiki/Code-Generation-Modes/

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
                          // 0 means no typed arrays are used.
                          // 1 has two heaps, IHEAP (int32) and FHEAP (double),
                          // and addresses there are a match for normal addresses. This is deprecated.
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

var OPENAL_DEBUG = 0; // Print out debugging information from our OpenAL implementation.

var GL_DEBUG = 0; // Print out all calls into WebGL. As with LIBRARY_DEBUG, you can set a runtime
                  // option, in this case GL.debug.
var GL_TESTING = 0; // When enabled, sets preserveDrawingBuffer in the context, to allow tests to work (but adds overhead)
var GL_MAX_TEMP_BUFFER_SIZE = 2097152; // How large GL emulation temp buffers are
var GL_UNSAFE_OPTS = 1; // Enables some potentially-unsafe optimizations in GL emulation code
var FULL_ES2 = 0; // Forces support for all GLES2 features, not just the WebGL-friendly subset.
var FORCE_GL_EMULATION = 0; // Forces inclusion of full GL emulation code.

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

var NAMED_GLOBALS = 0; // If 1, we use global variables for globals. Otherwise
                       // they are referred to by a base plus an offset (called an indexed global),
                       // saving global variables but adding runtime overhead.

var EXPORTED_FUNCTIONS = ['_main']; // Functions that are explicitly exported. These functions are kept alive
                                    // through LLVM dead code elimination, and also made accessible outside of
                                    // the generated code even after running closure compiler (on "Module").
                                    // Note the necessary prefix of "_".
var EXPORT_ALL = 0; // If true, we export all the symbols
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

var BUILD_AS_SHARED_LIB = 0; // Whether to build the code as a shared library
                             // 0 here means this is not a shared lib: It is a main file.
                             // All shared library options (1 and 2) are currently deprecated XXX
                             // 1 means this is a normal shared lib, load it with dlopen()
                             // 2 means this is a shared lib that will be linked at runtime,
                             //   which means it will insert its functions into
                             //   the global namespace. See STATIC_LIBS_TO_LOAD.
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

var ASM_JS = 0; // If 1, generate code in asm.js format. XXX This is highly experimental,
                // and will not work on most codebases yet. It is NOT recommended that you
                // try this yet.

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
// If you the headers or use different ones, you will need to override
// this.
var C_DEFINES = {'SI_MESGQ': '5',
   'M_SQRTPI': '1.77245385091',
   '_NL_MESSAGES_CODESET': '86',
   'SIGRTMIN': '27',
   'math_errhandling': '1',
   'M_LOG10E': '0.434294481903',
   '_S_IFMT': '0170000',
   '_CS_V7_ENV': '20',
   'USHRT_MAX': '65535',
   '_SC_XOPEN_LEGACY': '98',
   'HAVE_STDINT_H': '1',
   '_SC_XOPEN_VERSION': '106',
   'F_UNLCK': '3',
   '_SC_BC_DIM_MAX': '58',
   'SDL_LOADSO_DLOPEN': '1',
   'S_IFDIR': '0040000',
   'HAVE_ALLOCA': '1',
   '_SC_BARRIERS': '56',
   '_IFDIR': '0040000',
   'SDL_JOYSTICK_DINPUT': '1',
   '_IFLNK': '0120000',
   '__long_double_t': "<type 'long'>",
   'DEFFILEMODE': '0000400',
   'HAVE_SSCANF': '1',
   '_FCREAT': '512',
   'HAVE_STRTOLL': '1',
   'SDL_VIDEO_OPENGL_ES': '1',
   'O_CREAT': '512',
   'SHRT_MAX': '32767',
   'SDL_JOYSTICK_ANDROID': '1',
   '_SC_NPROCESSORS_CONF': '9',
   '_XOPEN_ENH_I18N': '1',
   'F_DUPFD_CLOEXEC': '14',
   '_CS_POSIX_V6_LP64_OFF64_LIBS': '11',
   '_POSIX_SHARED_MEMORY_OBJECTS': '200112',
   'ABDAY_7': '20',
   'ABDAY_6': '19',
   'ABDAY_5': '18',
   'ABDAY_4': '17',
   'ABDAY_3': '16',
   'M_PI': '3.14159265359',
   'ABDAY_1': '14',
   '_PC_REC_MIN_XFER_SIZE': '18',
   '_SC_V6_ILP32_OFFBIG': '93',
   'SIGSTOP': '17',
   '_M_LN2': '0.69314718056',
   'F_UNLKSYS': '4',
   'PTHREAD_CREATE_JOINABLE': '1',
   'SDL_VIDEO_OPENGL_GLX': '1',
   'M_PI_2': '1.57079632679',
   '_SC_MEMLOCK': '24',
   'M_PI_4': '0.785398163397',
   'FEXLOCK': '256',
   '_FNDELAY': '16384',
   'SIGEV_NONE': '1',
   'SIGWINCH': '28',
   'UTIME_NOW': '-2',
   '_SC_THREADS': '42',
   '__INT_MAX__': '2147483647',
   '_XBS5_LP64_OFF64': '-1',
   '_CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS': '6',
   '___int_least32_t_defined': '1',
   '_POSIX_MAPPED_FILES': '200112',
   'HAVE_FREE': '1',
   'M_E': '2.71828182846',
   'SIGTRAP': '5',
   '_SC_SS_REPL_MAX': '78',
   '_PC_SOCK_MAXBUF': '100',
   '_SC_THREAD_KEYS_MAX': '38',
   'O_RDWR': '2',
   '__LARGE64_FILES': '1',
   '_POSIX_V6_LP64_OFF64': '-1',
   'HAVE_COSF': '1',
   '_SC_2_PBS': '113',
   'FNOCTTY': '32768',
   '_SC_TRACE_INHERIT': '86',
   'PTHREAD_PRIO_NONE': '0',
   '_SC_REGEXP': '72',
   '_CS_POSIX_V6_LP64_OFF64_CFLAGS': '9',
   '_SC_DELAYTIMER_MAX': '37',
   'HAVE_SINF': '1',
   '_POSIX_RAW_SOCKETS': '200112',
   '___int64_t_defined': '1',
   'S_IFREG': '0100000',
   'SIGCLD': '20',
   'F_GETLK64': '20',
   '_IFCHR': '0020000',
   'S_IRWXG': '0000040',
   'SDL_VIDEO_DRIVER_ANDROID': '1',
   'POLLHUP': '16',
   'S_IFMT': '0170000',
   'RADIXCHAR': '50',
   'HAVE_UNSETENV': '1',
   '_S_IEXEC': '0000100',
   '_SC_XOPEN_CRYPT': '96',
   'M_LN10': '2.30258509299',
   'S_IRWXU': '0000400',
   'OPTIONAL_ARG': '2',
   '_PC_CHOWN_RESTRICTED': '6',
   'CRNCYSTR': '56',
   'SIZEOF_VOIDP': '4',
   'SCHAR_MAX': '127',
   'S_BLKSIZE': '1024',
   'SDL_JOYSTICK_NDS': '1',
   '_SC_CLK_TCK': '2',
   'AM_STR': '5',
   '__BUFSIZ__': '16',
   'ALT_DIGITS': '49',
   'HAVE_SIGNAL_H': '1',
   'HAVE_ATAN': '1',
   '_NL_CTYPE_MB_CUR_MAX': '85',
   '_REENT_SIGNAL_SIZE': '24',
   'SDL_AUDIO_DRIVER_DSOUND': '1',
   'F_RSETLKW': '13',
   'HAVE_STRTOULL': '1',
   '___int16_t_defined': '1',
   'SIGXCPU': '24',
   '_SC_MQ_PRIO_MAX': '14',
   '_FTRUNC': '1024',
   '__MACOSX__': '1',
   'SDL_LOADSO_WINDOWS': '1',
   'SDL_VIDEO_DRIVER_X11_XINERAMA': '1',
   'MALLOC_ALIGNMENT': '16',
   'PTHREAD_CREATE_DETACHED': '0',
   '_POSIX2_VERSION': '200112',
   '_O_CREAT': '512',
   'PM_STR': '6',
   '_PC_POSIX_SECURITY': '91',
   'HAVE_INTTYPES_H': '1',
   '_SC_2_LOCALEDEF': '112',
   '_SC_STREAM_MAX': '100',
   'HAVE_MEMCMP': '1',
   '_CS_POSIX_V7_ILP32_OFF32_LIBS': '3',
   '_POSIX2_C_BIND': '200112',
   '_POSIX_VERSION': '200112',
   'S_IFIFO': '0010000',
   'SDL_VIDEO_DRIVER_X11_XSCRNSAVER': '1',
   'SCHED_FIFO': '1',
   'SDL_HAPTIC_DISABLED': '1',
   'M_LN2LO': '1.90821492927e-10',
   'MON_10': '30',
   '_CS_XBS5_ILP32_OFF32_LIBS': '3',
   'O_SYNC': '8192',
   '_CS_POSIX_V6_ILP32_OFFBIG_LIBS': '7',
   'YESEXPR': '52',
   '_PC_PATH_MAX': '4',
   '_SC_SPORADIC_SERVER': '77',
   'SDL_POWER_UIKIT': '1',
   '_POSIX_SYNCHRONIZED_IO': '200112',
   'SIGXFSZ': '25',
   '_SC_NPROCESSORS_ONLN': '10',
   '_CS_POSIX_V6_LPBIG_OFFBIG_LIBS': '15',
   '_PC_MAX_INPUT': '2',
   'F_TLOCK': '2',
   'REQUIRED_ARG': '1',
   '_SC_VERSION': '7',
   'HUGE_VAL': 'inf',
   'AT_EACCESS': '1',
   'ABMON_3': '35',
   'ABMON_2': '34',
   'ABMON_1': '33',
   '_SC_THREAD_ROBUST_PRIO_PROTECT': '123',
   'ABMON_7': '39',
   'ABMON_6': '38',
   'ABMON_5': '37',
   'ABMON_4': '36',
   'S_IWUSR': '0000200',
   'ABMON_9': '41',
   'ABMON_8': '40',
   'UNDERFLOW': '4',
   '_SC_AIO_MAX': '35',
   'ERA': '45',
   '_CS_XBS5_ILP32_OFFBIG_LIBS': '7',
   'S_IXUSR': '0000100',
   '_SC_THREAD_PRIO_INHERIT': '46',
   '__HPUX__': '1',
   'M_2_PI': '0.636619772368',
   '_O_RDWR': '2',
   '_PC_2_SYMLINKS': '13',
   'POSIX_FADV_DONTNEED': '135',
   'SIG_BLOCK': '1',
   'SDL_VIDEO_RENDER_NDS': '0',
   '_O_WRONLY': '1',
   '_CS_XBS5_LP64_OFF64_CFLAGS': '9',
   '__OS2__': '1',
   'MON_2': '22',
   'MON_3': '23',
   '_POSIX_PRIORITY_SCHEDULING': '200112',
   'MON_1': '21',
   'MON_6': '26',
   'MON_7': '27',
   'MON_4': '24',
   'MON_5': '25',
   '_SC_SPAWN': '75',
   'MON_8': '28',
   'MON_9': '29',
   '_CS_POSIX_V6_ILP32_OFF32_LDFLAGS': '2',
   '_SC_TRACE_EVENT_NAME_MAX': '85',
   'SA_SIGINFO': '2',
   '_FBINARY': '65536',
   'PTHREAD_PRIO_PROTECT': '2',
   'POLLERR': '8',
   'SIGVTALRM': '26',
   'O_BINARY': '65536',
   '_REENT_EMERGENCY_SIZE': '25',
   'S_IEXEC': '0000100',
   '_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS': '13',
   'SIGEV_THREAD': '3',
   'ITIMER_VIRTUAL': '1',
   'HAVE_ATOI': '1',
   '_SC_TRACE_SYS_MAX': '89',
   '_POSIX_NO_TRUNC': '1',
   'HAVE_ATOF': '1',
   '__RISCOS__': '1',
   '_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS': '17',
   'HAVE_ATAN2': '1',
   'HAVE_PUTENV': '1',
   'SDL_AUDIO_DRIVER_ANDROID': '1',
   'F_SETFL': '4',
   'HAVE_GCC_ATOMICS': '1',
   '_CS_POSIX_V7_THREADS_CFLAGS': '18',
   '_SC_AIO_PRIO_DELTA_MAX': '36',
   '_POSIX2_C_DEV': '200112',
   '_SC_MONOTONIC_CLOCK': '69',
   '_POSIX_THREAD_SPORADIC_SERVER': '1',
   '_FNOINHERIT': '262144',
   'SDL_HAPTIC_NDS': '1',
   '_SC_XOPEN_ENH_I18N': '97',
   'SIGPROF': '27',
   'F_SETLKW64': '22',
   'HAVE__STRREV': '1',
   '_O_APPEND': '8',
   '_FDEFER': '32',
   'SDL_VIDEO_DRIVER_X11_XINPUT': '1',
   'CLOCK_DISALLOWED': '0',
   'SDL_VIDEO_DRIVER_X11': '1',
   '_SC_MEMORY_PROTECTION': '26',
   'HAVE_STDIO_H': '1',
   'LONG_MAX': '2147483647',
   'no_argument': '0',
   '__NINTENDODS__': '1',
   'F_OK': '0',
   'SDL_ASSEMBLY_ROUTINES': '1',
   'FAPPEND': '8',
   'SA_RESETHAND': '8',
   'FREAD': '1',
   '_SC_SPIN_LOCKS': '76',
   'SDL_VIDEO_DRIVER_COCOA': '1',
   'M_LN2HI': '0.693147180369',
   '_LIBC_LIMITS_H_': '1',
   'S_IFSOCK': '0140000',
   'SDL_AUDIO_DRIVER_COREAUDIO': '1',
   'HAVE_MALLOC': '1',
   '_POSIX_DEVCTL_DIRECTION': '1',
   'ABDAY_2': '15',
   'HAVE_STRTOL': '1',
   'SDL_VIDEO_DRIVER_DUMMY': '1',
   '_PC_LINK_MAX': '0',
   '_POSIX_THREAD_PRIO_PROTECT': '1',
   'HAVE_STRTOD': '1',
   '__DREAMCAST__': '1',
   'NOEXPR': '53',
   'FEXCL': '2048',
   '_SC_FSYNC': '22',
   '_SC_GETGR_R_SIZE_MAX': '50',
   '_POSIX_THREAD_PROCESS_SHARED': '200112',
   'HAVE_QSORT': '1',
   '_ATEXIT_SIZE': '32',
   '_SC_TRACE_NAME_MAX': '88',
   '_SC_BC_BASE_MAX': '57',
   '__WIN32__': '1',
   '_LIMITS_H': '1',
   'PTHREAD_STACK_MIN': '200',
   'O_APPEND': '8',
   '_CS_XBS5_LP64_OFF64_LINTFLAGS': '12',
   '_SC_XOPEN_STREAMS': '104',
   'HAVE_CALLOC': '1',
   'HAVE_CTYPE_H': '1',
   '_SC_GETPW_R_SIZE_MAX': '51',
   '_POSIX_ASYNCHRONOUS_IO': '1',
   'UCHAR_MAX': '255',
   '__BSDI__': '1',
   '_SC_PAGE_SIZE': '8',
   'SDL_THREADS_DISABLED': '1',
   '_SC_XBS5_ILP32_OFFBIG': '93',
   'S_IFBLK': '0060000',
   '_S_IFIFO': '0010000',
   'T_FMT_AMPM': '4',
   '_POSIX_SEMAPHORES': '200112',
   'HAVE_SCALBN': '1',
   '__NETBSD__': '1',
   'NBBY': '8',
   'SDL_AUDIO_DRIVER_XAUDIO2': '1',
   'SIGEMT': '7',
   '_POSIX_FSYNC': '200112',
   'F_SETLKW': '9',
   '_MB_EXTENDED_CHARSETS_WINDOWS': '1',
   'SIGALRM': '14',
   'SDL_VIDEO_DRIVER_UIKIT': '1',
   '___int32_t_defined': '1',
   'POLLNVAL': '4',
   '_SC_OPEN_MAX': '4',
   'CHAR_BIT': '8',
   '_N_LISTS': '30',
   '_SC_2_FORT_RUN': '111',
   'PTHREAD_MUTEX_DEFAULT': '3',
   'HAVE_STDARG_H': '1',
   '_POSIX_REGEXP': '1',
   '_SC_RE_DUP_MAX': '73',
   'PTHREAD_PROCESS_PRIVATE': '0',
   '_S_IFREG': '0100000',
   '_SC_THREAD_THREADS_MAX': '40',
   '_SC_THREAD_PRIO_PROTECT': '47',
   'SDL_THREAD_WINDOWS': '1',
   '_SC_2_PBS_CHECKPOINT': '115',
   'M_1_PI': '0.318309886184',
   '_PC_POSIX_PERMISSIONS': '90',
   '_SC_TIMERS': '33',
   'MON_11': '31',
   'MON_12': '32',
   'CLOCK_DISABLED': '0',
   '_SC_XBS5_LPBIG_OFFBIG': '95',
   '_POSIX_SPIN_LOCKS': '200112',
   '_FREAD': '1',
   'HAVE_SYSCONF': '1',
   '_SC_SHARED_MEMORY_OBJECTS': '199',
   'F_RDLCK': '1',
   'F_GETFD': '1',
   'MAC_OS_X_VERSION_10_4': '1040',
   'AT_SYMLINK_NOFOLLOW': '2',
   '_PC_ALLOC_SIZE_MIN': '15',
   '_POSIX_C_SOURCE': '2',
   '_SC_READER_WRITER_LOCKS': '71',
   'HAVE_STRING_H': '1',
   'SI_USER': '1',
   '_SC_MEMLOCK_RANGE': '25',
   '_SC_PRIORITY_SCHEDULING': '101',
   'optional_argument': '2',
   'T_FMT': '3',
   'LACKS_SYS_MMAN_H': '1',
   'MAC_OS_X_VERSION_10_5': '1050',
   '_PC_VDISABLE': '8',
   'SDL_VIDEO_DRIVER_X11_XSHAPE': '1',
   'THOUSEP': '51',
   'O_NOINHERIT': '262144',
   'PTHREAD_PROCESS_SHARED': '1',
   '_SC_TRACE_EVENT_FILTER': '84',
   'ERA_T_FMT': '48',
   '_SC_THREAD_ATTR_STACKADDR': '43',
   '_SC_LOGIN_NAME_MAX': '52',
   'M_LOG2E': '1.44269504089',
   'ITIMER_PROF': '2',
   'HAVE_LOG': '1',
   '_SC_2_C_BIND': '108',
   'FNONBIO': '16384',
   '_PC_NO_TRUNC': '7',
   'F_RSETLK': '11',
   '_SC_V7_ILP32_OFF32': '92',
   '_FAPPEND': '8',
   'PTHREAD_EXPLICIT_SCHED': '2',
   '_FNBIO': '4096',
   'HAVE_SYSCTLBYNAME': '1',
   'F_CNVT': '12',
   '_SC_SHELL': '74',
   '_SC_V6_LP64_OFF64': '94',
   'SCHED_OTHER': '0',
   '_CS_GNU_LIBC_VERSION': '42',
   '_SC_SEM_VALUE_MAX': '17',
   'S_ENFMT': '0002000',
   '_SC_MQ_OPEN_MAX': '13',
   'SDL_JOYSTICK_DISABLED': '1',
   '_POSIX_ADVISORY_INFO': '200112',
   'SIGABRT': '6',
   '_CS_POSIX_V7_ILP32_OFF32_CFLAGS': '1',
   '_CS_XBS5_ILP32_OFF32_CFLAGS': '1',
   '_MB_EXTENDED_CHARSETS_ISO': '1',
   '_SC_HOST_NAME_MAX': '65',
   '_SC_THREAD_STACK_MIN': '39',
   '_SC_TIMEOUTS': '82',
   'POLLOUT': '2',
   '_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS': '16',
   '_SC_CHILD_MAX': '1',
   '__RAND_MAX': '2147483647',
   'SDL_VIDEO_DRIVER_NDS': '1',
   '_POSIX_THREAD_ATTR_STACKADDR': '1',
   'SIGFPE': '8',
   'NL_ARGMAX': '32',
   '_SC_2_PBS_MESSAGE': '117',
   'TIMER_ABSTIME': '4',
   '_NL_CTYPE_CODESET_NAME': '0',
   '_SC_2_C_DEV': '109',
   '_SC_TIMER_MAX': '19',
   'S_IXOTH': '0000001',
   'FP_ZERO': '2',
   'SING': '2',
   'M_INVLN2': '1.44269504089',
   'SDL_TIMERS_DISABLED': '1',
   'M_TWOPI': '6.28318530718',
   '_PC_REC_XFER_ALIGN': '19',
   '_NL_TIME_DATE_FMT': '84',
   '_SC_REALTIME_SIGNALS': '29',
   '_POSIX2_RE_DUP_MAX': '255',
   'CLOCKS_PER_SEC': '1000',
   '_READ_WRITE_RETURN_TYPE': "<type 'int'>",
   'ERA_D_T_FMT': '47',
   'SIG_UNBLOCK': '2',
   '_CS_XBS5_ILP32_OFFBIG_LDFLAGS': '6',
   '_FSHLOCK': '128',
   'CLK_TCK': '1000',
   'D_FMT': '2',
   'SDL_VIDEO_OPENGL_CGL': '1',
   '_POSIX_SPAWN': '1',
   '_XBS5_ILP32_OFF32': '-1',
   '_SC_THREAD_PRIO_CEILING': '47',
   'SCHED_SPORADIC': '4',
   '_PC_ASYNC_IO': '9',
   'SI_TIMER': '3',
   'DAY_2': '8',
   'DAY_3': '9',
   'DAY_1': '7',
   'DAY_6': '12',
   'DAY_7': '13',
   'DAY_4': '10',
   'DAY_5': '11',
   'F_GETFL': '3',
   'HAVE_STRNCMP': '1',
   'AT_REMOVEDIR': '8',
   'SDL_THREAD_PTHREAD_RECURSIVE_MUTEX': '1',
   'PATH_MAX': '4096',
   '_POSIX_TIMEOUTS': '1',
   '_SC_MAPPED_FILES': '23',
   '__IRIX__': '1',
   'HAVE_INDEX': '1',
   'HAVE__LTOA': '1',
   '_SC_NGROUPS_MAX': '3',
   '__QNXNTO__': '1',
   '_FSYNC': '8192',
   'MATH_ERRNO': '1',
   '_POSIX_SAVED_IDS': '1',
   'SDL_POWER_MACOSX': '1',
   '_SC_SEMAPHORES': '30',
   '__FILENAME_MAX__': '255',
   'SIGTSTP': '18',
   'F_ULOCK': '0',
   'HAVE_COS': '1',
   '__LONG_MAX__': '2147483647',
   'F_WRLCK': '2',
   '_POSIX_JOB_CONTROL': '1',
   'FLT_EVAL_METHOD': '0',
   '_XOPEN_SHM': '1',
   '_POSIX_CHOWN_RESTRICTED': '1',
   'F_SETLK64': '21',
   '_SC_TRACE_LOG': '87',
   'HAVE_ITOA': '1',
   'SIGILL': '4',
   '_FNONBLOCK': '16384',
   '__OPENBSD__': '1',
   '_POSIX_TIMERS': '1',
   'FNDELAY': '16384',
   'FD_CLOEXEC': '1',
   'POLLIN': '1',
   '_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS': '14',
   '_POSIX_THREAD_CPUTIME': '1',
   'F_LOCK': '1',
   '_FLOAT_ARG': "<type 'float'>",
   'REQ_ARG': '1',
   '_CS_XBS5_ILP32_OFF32_LINTFLAGS': '4',
   '_SC_THREAD_DESTRUCTOR_ITERATIONS': '53',
   'M_LN2': '0.69314718056',
   'UINT_MAX': '2147483647',
   'HAVE_STRDUP': '1',
   'SIG_SETMASK': '0',
   '_SC_BC_STRING_MAX': '60',
   '_CS_XBS5_LPBIG_OFFBIG_LIBS': '15',
   '_SC_XOPEN_UUCP': '124',
   '_SC_2_SW_DEV': '119',
   'FDEFER': '32',
   'FP_NAN': '0',
   'F_SETOWN': '6',
   'SDL_LOADSO_DISABLED': '1',
   'CHAR_MIN': '-128',
   'PTHREAD_SCOPE_SYSTEM': '1',
   'HAVE_SETENV': '1',
   'HAVE_STRNCASECMP': '1',
   '_POSIX_V6_LPBIG_OFFBIG': '-1',
   '_S_IWRITE': '0000200',
   '_S_IFDIR': '0040000',
   '_SC_ARG_MAX': '0',
   '_SC_THREAD_PRIORITY_SCHEDULING': '45',
   'F_GETLK': '7',
   'SIGTTIN': '21',
   '_CS_POSIX_V7_WIDTH_RESTRICTED_ENVS': '17',
   '_POSIX_SPORADIC_SERVER': '1',
   '_SC_THREAD_CPUTIME': '80',
   '_POSIX_V6_ILP32_OFF32': '-1',
   '_CS_POSIX_V7_ILP32_OFFBIG_LIBS': '7',
   '_CS_POSIX_V6_ILP32_OFF32_LIBS': '3',
   '_SC_SYNCHRONIZED_IO': '32',
   '_UNIX98_THREAD_MUTEX_ATTRIBUTES': '1',
   '_POSIX_REALTIME_SIGNALS': '200112',
   '__SIGLASTNOTRT': '31',
   'ERA_D_FMT': '46',
   'HAVE_RINDEX': '1',
   'OPT_ARG': '2',
   'SDL_HAPTIC_IOKIT': '1',
   '_FASYNC': '64',
   '_CS_POSIX_V6_ILP32_OFF32_CFLAGS': '1',
   'NOSTR': '55',
   '_POSIX_MONOTONIC_CLOCK': '200112',
   'SIGPOLL': '23',
   'S_ISGID': '0002000',
   'FP_INFINITE': '1',
   'ULONG_MAX': '2147483647',
   '__SIGFIRSTNOTRT': '1',
   'AT_SYMLINK_FOLLOW': '4',
   'FSYNC': '8192',
   '__USE_XOPEN2K': '1',
   'SDL_VIDEO_RENDER_OGL_ES': '1',
   '_CS_XBS5_ILP32_OFFBIG_CFLAGS': '5',
   '_IFSOCK': '0140000',
   '_IFIFO': '0010000',
   'ARG_MAX': '4096',
   'SIGPIPE': '13',
   'HAVE__ULTOA': '1',
   '_O_EXCL': '2048',
   'O_TRUNC': '1024',
   'O_TEXT': '131072',
   '_POSIX_THREAD_PRIO_INHERIT': '1',
   '_XBS5_ILP32_OFFBIG': '1',
   'HAVE_MEMMOVE': '1',
   'STDERR_FILENO': '2',
   '_CS_XBS5_LPBIG_OFFBIG_CFLAGS': '13',
   '__LINUX__': '1',
   'PLOSS': '6',
   'S_IRWXO': '0000004',
   '_SC_V7_LP64_OFF64': '94',
   '_O_NOINHERIT': '262144',
   'D_MD_ORDER': '57',
   '_IFMT': '0170000',
   '_SC_SYMLOOP_MAX': '79',
   'MB_LEN_MAX': '1',
   'SDL_TIMER_WINDOWS': '1',
   '_SC_XOPEN_UNIX': '105',
   'M_IVLN10': '0.434294481903',
   'ALLPERMS': '0004000',
   'HAVE_STRSTR': '1',
   '__BEOS__': '1',
   'HAVE_GCC_SYNC_LOCK_TEST_AND_SET': '1',
   'F_SETFD': '2',
   'SIGUSR1': '30',
   'HAVE_SIN': '1',
   'SDL_VIDEO_DRIVER_X11_XRANDR': '1',
   'MAC_OS_X_VERSION_10_6': '1060',
   '___int8_t_defined': '1',
   'SIGKILL': '9',
   '_CS_POSIX_V7_ILP32_OFFBIG_CFLAGS': '5',
   'PTHREAD_MUTEX_RECURSIVE': '1',
   'SIGSEGV': '11',
   'M_LOG2_E': '0.69314718056',
   'FWRITE': '2',
   '_FEXCL': '2048',
   'SIGINT': '2',
   'HAVE_STRRCHR': '1',
   'H8300': '1',
   '_POSIX_MEMORY_PROTECTION': '200112',
   'FP_ILOGBNAN': '2147483647',
   '_SC_V7_LPBIG_OFFBIG': '95',
   '_SC_CLOCK_SELECTION': '61',
   '_PC_PRIO_IO': '10',
   'M_2_SQRTPI': '1.1283791671',
   'S_IROTH': '0000004',
   '_SC_MESSAGE_PASSING': '27',
   '_SC_V6_LPBIG_OFFBIG': '95',
   'SDL_AUDIO_DRIVER_DUMMY': '1',
   'HAVE_MEMCPY': '1',
   '_SC_EXPR_NEST_MAX': '64',
   'SDL_TIMER_WINCE': '1',
   'STDC_HEADERS': '1',
   '_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS': '5',
   'S_ISVTX': '0001000',
   'HAVE_STRLCPY': '1',
   'UTIME_OMIT': '-1',
   '_CS_POSIX_V7_THREADS_LDFLAGS': '19',
   'SDL_POWER_WINDOWS': '1',
   '_SC_PAGESIZE': '8',
   '_PC_REC_MAX_XFER_SIZE': '17',
   'SIGIOT': '6',
   'FASYNC': '64',
   '_SC_V7_ILP32_OFFBIG': '93',
   '_RAND48_MULT_0': '58989',
   '_RAND48_MULT_1': '57068',
   '_RAND48_MULT_2': '5',
   '_CS_POSIX_V6_LP64_OFF64_LDFLAGS': '10',
   'HAVE_STRTOUL': '1',
   'PTHREAD_MUTEX_NORMAL': '0',
   '_O_TRUNC': '1024',
   'W_OK': '2',
   'O_NONBLOCK': '16384',
   'R_OK': '4',
   '_IFBLK': '0060000',
   'FTRUNC': '1024',
   '__OSF__': '1',
   '_SC_XBS5_LP64_OFF64': '94',
   'STDIN_FILENO': '0',
   'HAVE_ABS': '1',
   'SDL_TIMER_NDS': '1',
   '_CS_POSIX_V7_LPBIG_OFFBIG_LIBS': '15',
   'SDL_AUDIO_DRIVER_DISK': '1',
   '_SC_SIGQUEUE_MAX': '18',
   'M_3PI_4': '2.35619449019',
   'HAVE_STRCMP': '1',
   '_POSIX_MESSAGE_PASSING': '200112',
   'S_ISUID': '0004000',
   'SIGLOST': '29',
   '_POSIX_THREAD_PRIORITY_SCHEDULING': '200112',
   'SDL_VIDEO_RENDER_OGL_ES2': '1',
   '__FREEBSD__': '1',
   '_SC_TZNAME_MAX': '20',
   '_O_RAW': '65536',
   '_CS_PATH': '0',
   '_POSIX_BARRIERS': '200112',
   'SDL_ALTIVEC_BLITTERS': '1',
   'SEEK_SET': '0',
   '_POSIX_THREAD_SAFE_FUNCTIONS': '200112',
   'S_IREAD': '0000400',
   '_LONG_LONG_TYPE': "<type 'long'>",
   '___int_least8_t_defined': '1',
   'INT_MAX': '2147483647',
   '_POSIX_V6_ILP32_OFFBIG': '1',
   '_SC_PHYS_PAGES': '11',
   'HAVE_BCOPY': '1',
   '_PC_MAX_CANON': '1',
   'HAVE_STRICMP': '1',
   '_CS_XBS5_LPBIG_OFFBIG_LDFLAGS': '14',
   '_SC_THREAD_SAFE_FUNCTIONS': '49',
   'SIGRTMAX': '31',
   'S_IXGRP': '0000010',
   'HAVE_GETENV': '1',
   '_XBS5_LPBIG_OFFBIG': '-1',
   '_PC_NAME_MAX': '3',
   'O_EXCL': '2048',
   '_SC_XOPEN_SHM': '103',
   'S_IWGRP': '0000020',
   '_SC_TRACE_USER_EVENT_MAX': '90',
   'SDL_VIDEO_RENDER_D3D': '1',
   'HAVE__STRNICMP': '1',
   'M_SQRT1_2': '0.707106781187',
   '_SC_AVPHYS_PAGES': '12',
   '_SC_RAW_SOCKETS': '70',
   'O_RDONLY': '0',
   '_DATE_FMT': '84',
   'HAVE_STRCASECMP': '1',
   '__SOLARIS__': '1',
   '_SC_RTSIG_MAX': '15',
   '_POSIX_DEVICE_CONTROL': '1',
   'CLOCK_ENABLED': '1',
   '_NULL': '0',
   '_SC_PRIORITIZED_IO': '28',
   '_O_TEXT': '131072',
   'SDL_VIDEO_DRIVER_X11_XVIDMODE': '1',
   'SIGBUS': '10',
   'CODESET': '0',
   'CHAR_MAX': '127',
   'SDL_POWER_NINTENDODS': '1',
   'SIGSYS': '12',
   '_PC_REC_INCR_XFER_SIZE': '16',
   'S_IRUSR': '0000400',
   '_PC_FILESIZEBITS': '12',
   '_SC_XBS5_ILP32_OFF32': '92',
   'HAVE_MATH_H': '1',
   'HAVE_SQRT': '1',
   'SIGURG': '16',
   '_POSIX_THREAD_ATTR_STACKSIZE': '200112',
   '_CS_POSIX_V7_LP64_OFF64_LIBS': '11',
   '_CS_GNU_LIBPTHREAD_VERSION': '43',
   'HAVE_M_PI': '1',
   '_REENT_ASCTIME_SIZE': '26',
   '_SC_2_PBS_LOCATE': '116',
   '_SC_V6_ILP32_OFF32': '92',
   'SIGCHLD': '20',
   'SHRT_MIN': '-32768',
   '__HAIKU__': '1',
   'PTHREAD_MUTEX_ERRORCHECK': '2',
   '_PC_SYNC_IO': '11',
   'SDL_VIDEO_OPENGL': '1',
   'FP_NORMAL': '4',
   '_SC_2_UPE': '120',
   'HAVE_POW': '1',
   '_SC_SEM_NSEMS_MAX': '16',
   '__ANDROID__': '1',
   '_SC_IOV_MAX': '66',
   'S_IRGRP': '0000040',
   'YESSTR': '54',
   'HAVE_ALLOCA_H': '1',
   'S_IFCHR': '0020000',
   '_POSIX_MEMLOCK': '1',
   '_SC_TRACE': '83',
   '_POSIX_INTERRUPT_CONTROL': '1',
   '_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS': '6',
   'PTHREAD_SCOPE_PROCESS': '0',
   '__GNUC_VA_LIST': '1',
   'HAVE_FABS': '1',
   '_CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS': '14',
   'TLOSS': '5',
   '_TIME_T_': "<type 'long'>",
   'DOMAIN': '1',
   'HAVE_REALLOC': '1',
   'HAVE_STRLEN': '1',
   '__IPHONEOS__': '1',
   '_POINTER_INT': "<type 'long'>",
   'O_NOCTTY': '32768',
   'PTHREAD_PRIO_INHERIT': '1',
   '_SC_THREAD_SPORADIC_SERVER': '81',
   'O_ACCMODE': '3',
   '_SC_LINE_MAX': '68',
   'D_T_FMT': '1',
   '_RAND48_SEED_1': '43981',
   '_RAND48_SEED_0': '13070',
   '_RAND48_SEED_2': '4660',
   'HAVE_STRLCAT': '1',
   'HAVE_SETJMP': '1',
   'SDL_AUDIO_DRIVER_COREAUDIOIPHONE': '1',
   'FOPEN': '-1',
   'F_RGETLK': '10',
   'F_DUPFD': '0',
   '_S_IFCHR': '0020000',
   'SDL_IPHONE_KEYBOARD': '1',
   '_POSIX_IPV6': '200112',
   '_XOPEN_VERSION': '600',
   'HAVE_FLOOR': '1',
   '_CS_XBS5_LP64_OFF64_LDFLAGS': '10',
   '_FNOCTTY': '32768',
   '_IFREG': '0100000',
   'FP_SUBNORMAL': '3',
   'SA_NOCLDSTOP': '1',
   'HAVE_STDDEF_H': '1',
   'NSIG': '32',
   'HAVE__STRUPR': '1',
   'SDL_THREAD_PTHREAD': '1',
   'S_IWOTH': '0000002',
   'SEEK_END': '2',
   'SDL_ASSERT_LEVEL': '1',
   'SI_ASYNCIO': '4',
   'required_argument': '1',
   '_FWRITE': '2',
   'SCHED_RR': '2',
   '_SC_2_FORT_DEV': '110',
   'SA_NODEFER': '4',
   '_POSIX2_CHAR_TERM': '200112',
   'F_SETLK': '8',
   'SIGQUIT': '3',
   '_SC_ATEXIT_MAX': '55',
   '_POSIX_MEMLOCK_RANGE': '200112',
   '_SC_SAVED_IDS': '6',
   'PTHREAD_INHERIT_SCHED': '1',
   'SEEK_CUR': '1',
   'S_IFLNK': '0120000',
   '_S_IREAD': '0000400',
   '_FOPEN': '-1',
   'OVERFLOW': '3',
   '_POSIX_SHELL': '1',
   'FMARK': '16',
   '_POSIX2_UPE': '200112',
   'SDL_VIDEO_RENDER_OGL': '1',
   '_SC_2_PBS_TRACK': '118',
   '_POSIX_CPUTIME': '1',
   '_SC_THREAD_PROCESS_SHARED': '48',
   '_SC_JOB_CONTROL': '5',
   '_O_RDONLY': '0',
   'FNBIO': '4096',
   'O_CLOEXEC': '262144',
   '_CS_XBS5_LP64_OFF64_LIBS': '11',
   '_PC_TIMESTAMP_RESOLUTION': '20',
   '_POSIX_READER_WRITER_LOCKS': '200112',
   'ACCESSPERMS': '0000400',
   '_POSIX_PRIORITIZED_IO': '1',
   '_SC_IPV6': '67',
   'SDL_VIDEO_OPENGL_WGL': '1',
   '_CS_XBS5_ILP32_OFFBIG_LINTFLAGS': '8',
   'HAVE__STRICMP': '1',
   '_SC_ADVISORY_INFO': '54',
   'SCHAR_MIN': '-128',
   '_SC_XOPEN_REALTIME_THREADS': '102',
   'SIGEV_SIGNAL': '2',
   'HAVE_NANOSLEEP': '1',
   'O_WRONLY': '1',
   '_PC_SYMLINK_MAX': '14',
   '_SC_THREAD_ROBUST_PRIO_INHERIT': '122',
   'X_OK': '1',
   '_CS_XBS5_ILP32_OFF32_LDFLAGS': '2',
   'SIGTERM': '15',
   '_SC_COLL_WEIGHTS_MAX': '62',
   '_CS_POSIX_V7_ILP32_OFF32_LDFLAGS': '2',
   'NO_ARG': '0',
   '_CS_POSIX_V7_LP64_OFF64_CFLAGS': '9',
   'S_IWRITE': '0000200',
   '_FEXLOCK': '256',
   '_SC_XOPEN_REALTIME': '99',
   'SIGPWR': '19',
   'SDL_AUDIO_DRIVER_WINMM': '1',
   'HAVE_STRCHR': '1',
   '_PC_PIPE_BUF': '5',
   'SDL_HAPTIC_DINPUT': '1',
   'SIGHUP': '1',
   'F_GETOWN': '5',
   'CLOCK_ALLOWED': '1',
   'HAVE_MEMSET': '1',
   'SIGUSR2': '31',
   '_SC_2_PBS_ACCOUNTING': '114',
   'F_TEST': '3',
   'HAVE_VSNPRINTF': '1',
   'ITIMER_REAL': '0',
   'HAVE_SNPRINTF': '1',
   'HAVE_SYS_TYPES_H': '1',
   'HAVE_COPYSIGN': '1',
   '_CLOCKS_PER_SEC_': '1000',
   'SDL_HAPTIC_DUMMY': '1',
   'SIGCONT': '19',
   'NULL': '0',
   'FSHLOCK': '128',
   'STDOUT_FILENO': '1',
   '__AIX__': '1',
   'SDL_JOYSTICK_IOKIT': '1',
   '_SC_THREAD_ATTR_STACKSIZE': '44',
   'SIGIO': '23',
   'HAVE_CEIL': '1',
   'HAVE__STRLWR': '1',
   'HAVE_SIGACTION': '1',
   '_SC_CPUTIME': '63',
   '_SC_2_VERSION': '121',
   '_O_BINARY': '65536',
   '___int_least16_t_defined': '1',
   '_FTEXT': '131072',
   '_POSIX2_SW_DEV': '200112',
   '_LARGEFILE64_SOURCE': '1',
   '_XOPEN_CRYPT': '1',
   'FD_SETSIZE': '64',
   'SDL_AUDIO_DRIVER_NDS': '1',
   '_FMARK': '16',
   '_SC_TYPED_MEMORY_OBJECTS': '91',
   '_SC_ASYNCHRONOUS_IO': '21',
   '_SC_2_CHAR_TERM': '107',
   '_SC_AIO_LISTIO_MAX': '34',
   'FCREAT': '512',
   '_RAND48_ADD': '11',
   'MAXPATHLEN': '1024',
   '_SC_BC_SCALE_MAX': '59',
   'SDL_IPHONE_MAX_GFORCE': '5.0',
   'MATH_ERREXCEPT': '2',
   'SDL_VIDEO_DRIVER_WINDOWS': '1',
   '_CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS': '13',
   'M_SQRT3': '1.73205080757',
   'M_SQRT2': '1.41421356237',
   '_POSIX_THREADS': '200112',
   'ABMON_12': '44',
   'ABMON_11': '43',
   'ABMON_10': '42',
   'SI_QUEUE': '2',
   'SDL_TIMER_UNIX': '1',
   'AT_FDCWD': '-2',
   'SIGTTOU': '22',
   '_CS_POSIX_V7_LP64_OFF64_LDFLAGS': '10',
   '_SC_TTY_NAME_MAX': '41',
   'AF_INET': '1',
   'AF_INET6': '6',
   'FIONREAD': '1',
   'SOCK_STREAM': '200',
   'IPPROTO_TCP': 1
};

