// Tuning
QUANTUM_SIZE = 4; // This is the size of an individual field in a structure. 1 would
                  // lead to e.g. doubles and chars both taking 1 memory address. This
                  // is a form of 'compressed' memory, with shrinking and stretching
                  // according to the type, when compared to C/C++. On the other hand
                  // the normal value of 4 means all fields take 4 memory addresses,
                  // as per the norm on a 32-bit machine.
                  //
                  // 1 is somewhat faster than 4, but dangerous.

CORRECT_SIGNS = 1; // Whether we make sure to convert unsigned values to signed values.
                   // Decreases performance with additional runtime checks. Might not be
                   // needed in some kinds of code.
                   // If equal to 2, done on a line-by-line basis according to
                   // CORRECT_SIGNS_LINES, correcting only the specified lines.
                   // If equal to 3, correcting all *but* the specified lines
CHECK_SIGNS = 0; // Runtime errors for signing issues that need correcting.
                 // It is recommended to use this in
                 // order to find if your code needs CORRECT_SIGNS. If you can get your
                 // code to run without CORRECT_SIGNS, it will run much faster

ASSERTIONS = 1; // Whether we should add runtime assertions, for example to
                // check that each allocation to the stack does not
                // exceed it's size, whether all allocations (stack and static) are
                // of positive size, etc., whether we should throw if we encounter a bad __label__, i.e.,
                // if code flow runs into a fault

INVOKE_RUN = 1; // Whether we will call run(). Disable if you embed the generated
                // code in your own, and will call run() yourself at the right time
INIT_STACK = 1; // Whether to initialize memory on the stack to 0.
INIT_HEAP = 0; // Whether to initialize memory anywhere other than the stack to 0.
FAST_MEMORY = 2*1024*1024; // The amount of memory to initialize to 0. This ensures it will be
                           // in a flat array. This only matters in non-typed array builds.
TOTAL_MEMORY = 50*1024*1024; // The total amount of memory to use. This mainly matters in
                             // typed array builds - accessing memory about this value will
                             // return undefined values and lead to serious problems, and there
                             // is currently no warning about that!

// Code embetterments
OPTIMIZE = 0; // Optimize llvm operations into js commands
RELOOP = 0; // Recreate js native loops from llvm data
USE_TYPED_ARRAYS = 0; // Try to use typed arrays for the heap
                      // 1 has two heaps, IHEAP (int32) and FHEAP (double),
                      // and addresses there are a match for normal addresses. This wastes memory but can be fast.
                      // 2 is a single heap, accessible through views as int8, int32, etc. This saves memory but
                      // has more overhead of pointer calculations. It also is limited to storing doubles as floats,
                      // simply because double stores are not necessarily 64-bit aligned, and we can only access
                      // 64-bit aligned values with a 64-bit typed array. Likewise int64s are stored as int32's,
                      // which is potentially very dangerous!
                      // TODO: require compiling with -malign-double, which does align doubles
SKIP_STACK_IN_SMALL = 1; // When enabled, does not push/pop the stack at all in
                         // functions that have no basic stack usage. But, they
                         // may allocate stack later, and in a loop, this can be
                         // very bad. In particular, when debugging, printf()ing
                         // a lot can exhaust the stack very fast, with this option.

// Generated code debugging options
SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
               // If equal to 2, done on a line-by-line basis according to
               // SAFE_HEAP_LINES, checking only the specified lines.
               // If equal to 3, checking all *but* the specified lines. Note
               // that 3 is the option you usually want here.
SAFE_HEAP_LOG = 0; // Log out all SAFE_HEAP operations

LABEL_DEBUG = 0; // Print out labels and functions as we enter them
EXCEPTION_DEBUG = 1; // Print out exceptions in emscriptened code
LIBRARY_DEBUG = 0; // Print out when we enter a library call (library*.js)
DISABLE_EXCEPTION_CATCHING = 0; // Disables generating code to actually catch exceptions. If the code you
                                // are compiling does not actually rely on catching exceptions (but the
                                // compiler generates code for it, maybe because of stdlibc++ stuff),
                                // then this can make it much faster. If an exception actually happens,
                                // it will not be caught and the program will halt (so this will not
                                // introduce silent failures, which is good).
                                // TODO: Make this also remove cxa_begin_catch etc., optimize relooper
                                //       for it, etc. (perhaps do all of this as preprocessing on .ll?)
EXECUTION_TIMEOUT = -1; // Throw an exception after X seconds - useful to debug infinite loops
CHECK_OVERFLOWS = 0; // Add code that checks for overflows in integer math operations.
                     // There is currently not much to do to handle overflows if they occur.
                     // We can add code to simulate i32/i64 overflows in JS, but that would
                     // be very slow. It probably makes more sense to avoid overflows in
                     // C/C++ code. For example, if you have an int that you multiply by
                     // some factor, in order to get 'random' hash values - by taking
                     // that |value & hash_table_size| - then multiplying enough times will overflow.
                     // But instead, you can do |value = value & 30_BITS| in each iteration.
CHECK_SIGNED_OVERFLOWS = 0; // Whether to allow *signed* overflows - our correction for overflows generates signed
                            // values (since we use &). This means that we correct some things are not strictly overflows,
                            // and we cause them to be signed (which may lead to unnecessary unSign()ing later).
                            // With this enabled, we check signed overflows for CHECK_OVERFLOWS
CORRECT_OVERFLOWS = 1; // Experimental code that tries to prevent unexpected JS overflows in integer
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
CORRECT_ROUNDINGS = 1; // C rounds to 0 (-5.5 to -5, +5.5 to 5), while JS has no direct way to do that:
                       // Math.floor is to negative, ceil to positive. With CORRECT_ROUNDINGS,
                       // we will do slow but correct C rounding operations.

AUTO_OPTIMIZE = 0; // When run with the CHECK_* options, will not fail on errors. Instead, will
                   // keep a record of which checks succeeded and which failed. On shutdown, will
                   // print out that information. This is useful for knowing which lines need
                   // checking enabled and which do not, that is, this is a way to automate the
                   // generation of line data for CORRECT_*_LINES options

PROFILE = 0; // Enables runtime profiling. See test_profiling for a usage example.

EXPORTED_FUNCTIONS = ['_main']; // Functions that are explicitly exported, so they are guaranteed to
                                // be accessible outside of the generated code.

EXPORTED_GLOBALS = []; // Global non-function variables that are explicitly
                       // exported, so they are guaranteed to be
                       // accessible outside of the generated code.

INCLUDE_FULL_LIBRARY = 0; // Whether to include the whole library rather than just the
                          // functions used by the generated code. This is needed when
                          // dynamically loading modules that make use of runtime
                          // library functions that are not used in the main module.

// A set of defines, for example generated from your header files. This
// lets the emscripten libc (library.js) see the right values.
// The default value here has been generated from system/include. If you
// modify those files, or use different headers, you will need to override
// this.
C_DEFINES = {"M_SQRTPI":"1.77245385091","__RAND_MAX":"2147483647","__FILENAME_MAX__":"255","math_errhandling":"1","M_LOG10E":"0.434294481903","_M_LN2":"0.69314718056","__LONG_MAX__":"2147483647","_POSIX_JOB_CONTROL":"1","FLT_EVAL_METHOD":"0","__BUFSIZ__":"16","_XOPEN_SHM":"1","_POSIX_CHOWN_RESTRICTED":"1","_POSIX_THREAD_ATTR_STACKSIZE":"200112","LONG_MAX":"2147483647","_POSIX_TIMERS":"1","_POSIX2_C_DEV":"200112","_READ_WRITE_RETURN_TYPE":"<type 'int'>","_POSIX_THREAD_SPORADIC_SERVER":"1","SCHED_SPORADIC":"4","_REENT_ASCTIME_SIZE":"26","_POSIX_THREAD_CPUTIME":"1","SHRT_MIN":"-32768","PTHREAD_MUTEX_ERRORCHECK":"2","_LIBC_LIMITS_H_":"1","M_LN2":"0.69314718056","M_LN2HI":"0.693147180369","UINT_MAX":"2147483647","FP_NORMAL":"4","_POSIX_SHARED_MEMORY_OBJECTS":"200112","_POSIX_DEVCTL_DIRECTION":"1","_POSIX_MEMLOCK":"1","FP_NAN":"0","M_PI":"3.14159265359","_POSIX_THREAD_PRIO_PROTECT":"1","___int_least16_t_defined":"1","PTHREAD_SCOPE_SYSTEM":"1","_POSIX_INTERRUPT_CONTROL":"1","NBBY":"8","__LARGE64_FILES":"1","PTHREAD_SCOPE_PROCESS":"0","TLOSS":"5","PTHREAD_CREATE_JOINABLE":"1","M_PI_2":"1.57079632679","_TIME_T_":"<type 'long'>","DOMAIN":"1","SCHAR_MIN":"-128","M_PI_4":"0.785398163397","_POSIX_SPORADIC_SERVER":"1","CHAR_MAX":"127","_POINTER_INT":"<type 'long'>","PTHREAD_STACK_MIN":"200","USHRT_MAX":"65535","_POSIX_V6_ILP32_OFF32":"-1","UCHAR_MAX":"255","__INT_MAX__":"2147483647","_XBS5_LP64_OFF64":"-1","_UNIX98_THREAD_MUTEX_ATTRIBUTES":"1","_POSIX_REALTIME_SIGNALS":"200112","M_E":"2.71828182846","_RAND48_SEED_1":"43981","_RAND48_SEED_0":"13070","_RAND48_SEED_2":"4660","_POSIX_FSYNC":"200112","CHAR_MIN":"-128","_MB_EXTENDED_CHARSETS_WINDOWS":"1","FP_INFINITE":"1","_N_LISTS":"30","___int_least32_t_defined":"1","MB_LEN_MAX":"1","__USE_XOPEN2K":"1","PTHREAD_MUTEX_DEFAULT":"3","_POSIX_REGEXP":"1","PTHREAD_PROCESS_PRIVATE":"0","FP_SUBNORMAL":"3","_POSIX_DEVICE_CONTROL":"1","PTHREAD_PRIO_NONE":"0","___int64_t_defined":"1","_POSIX_THREAD_PRIO_INHERIT":"1","_POSIX_CPUTIME":"1","_XBS5_ILP32_OFFBIG":"1","_POSIX_SPIN_LOCKS":"200112","ARG_MAX":"4096","HUGE_VAL":"inf","PLOSS":"6","_POSIX2_VERSION":"200112","M_LN10":"2.30258509299","_POSIX_THREAD_SAFE_FUNCTIONS":"200112","_POSIX2_CHAR_TERM":"200112","SCHED_RR":"2","_POSIX_C_SOURCE":"2","_XOPEN_ENH_I18N":"1","M_IVLN10":"0.434294481903","_POSIX_SAVED_IDS":"1","_POSIX_MEMLOCK_RANGE":"200112","PTHREAD_INHERIT_SCHED":"1","___int16_t_defined":"1","_LIMITS_H":"1","OVERFLOW":"3","PTHREAD_PROCESS_SHARED":"1","___int8_t_defined":"1","_POSIX2_UPE":"200112","CHAR_BIT":"8","MALLOC_ALIGNMENT":"16","PTHREAD_MUTEX_RECURSIVE":"1","PTHREAD_CREATE_DETACHED":"0","M_LOG2_E":"0.69314718056","M_LOG2E":"1.44269504089","_POSIX_BARRIERS":"200112","H8300":"1","_POSIX_SHELL":"1","_POSIX_MEMORY_PROTECTION":"200112","FP_ILOGBNAN":"2147483647","_POSIX_RAW_SOCKETS":"200112","M_2_SQRTPI":"1.1283791671","PTHREAD_EXPLICIT_SCHED":"2","_POSIX_PRIORITIZED_IO":"1","PATH_MAX":"4096","_POSIX_THREAD_PROCESS_SHARED":"200112","_POSIX2_C_BIND":"200112","_POSIX_V6_LP64_OFF64":"-1","_POSIX_VERSION":"200112","_POSIX_SPAWN":"1","SCHED_FIFO":"1","M_LN2LO":"1.90821492927e-10","_POSIX_ADVISORY_INFO":"200112","_NULL":"0","_POSIX_V6_LPBIG_OFFBIG":"-1","_XOPEN_VERSION":"600","_POSIX_SYNCHRONIZED_IO":"200112","_MB_EXTENDED_CHARSETS_ISO":"1","_POSIX_MAPPED_FILES":"200112","SCHAR_MAX":"127","_POSIX_MONOTONIC_CLOCK":"200112","ULONG_MAX":"2147483647","SHRT_MAX":"32767","_RAND48_MULT_0":"58989","_RAND48_MULT_1":"57068","_RAND48_MULT_2":"5","PTHREAD_MUTEX_NORMAL":"0","NL_ARGMAX":"32","M_1_PI":"0.318309886184","UNDERFLOW":"4","PTHREAD_PRIO_INHERIT":"1","FP_ZERO":"2","SING":"2","___int32_t_defined":"1","M_INVLN2":"1.44269504089","M_2_PI":"0.636619772368","M_TWOPI":"3.14159265359","_POSIX_ASYNCHRONOUS_IO":"1","_POSIX2_RE_DUP_MAX":"255","M_3PI_4":"2.35619449019","_FLOAT_ARG":"<type 'float'>","_POSIX_MESSAGE_PASSING":"200112","_POSIX_THREAD_PRIORITY_SCHEDULING":"200112","SCHED_OTHER":"0","_XOPEN_CRYPT":"1","_ATEXIT_SIZE":"32","_POSIX2_SW_DEV":"200112","_POSIX_PRIORITY_SCHEDULING":"200112","_LARGEFILE64_SOURCE":"1","_REENT_SIGNAL_SIZE":"24","FD_SETSIZE":"64","_POSIX_SEMAPHORES":"200112","_XBS5_ILP32_OFF32":"-1","_POSIX_IPV6":"200112","_LONG_LONG_TYPE":"<type 'long'>","___int_least8_t_defined":"1","INT_MAX":"2147483647","_POSIX_V6_ILP32_OFFBIG":"1","PTHREAD_PRIO_PROTECT":"2","_RAND48_ADD":"11","_REENT_EMERGENCY_SIZE":"25","_POSIX_READER_WRITER_LOCKS":"200112","_XBS5_LPBIG_OFFBIG":"-1","_POSIX_NO_TRUNC":"1","_POSIX_TIMEOUTS":"1","_POSIX_THREAD_ATTR_STACKADDR":"1","M_SQRT3":"1.73205080757","M_SQRT2":"1.41421356237","_POSIX_THREADS":"200112","MATH_ERREXCEPT":"2","MATH_ERRNO":"1","M_SQRT1_2":"0.707106781187",
  F_GETLK64: 20,
  F_SETLK64: 21,
  F_SETLKW64: 22
};

SHOW_LABELS = 0; // Show labels in the generated code

BUILD_AS_SHARED_LIB = 0; // Whether to build the code as a shared library, which
                         // must be loaded dynamically using dlopen().

RUNTIME_TYPE_INFO = 0; // Whether to expose type info to the script at run time. This
                       // increases the size of the generated script, but allows you
                       // to more easily perform operations from handwritten JS on
                       // objects with structures etc.

FAKE_X86_FP80 = 0; // Replaces x86_fp80 with double. This loses precision. It is better,
                   // if you can, to get the original source code to build without x86_fp80
                   // (which is nonportable anyhow).

// Compiler debugging options
DEBUG_TAGS_SHOWING = [];
  // Some useful items:
  //    framework
  //    frameworkLines
  //    gconst
  //    types
  //    vars
  //    relooping
  //    unparsedFunctions
  //    metadata

