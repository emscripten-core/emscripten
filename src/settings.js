//"use strict";

// Tuning
var QUANTUM_SIZE = 4; // This is the size of an individual field in a structure. 1 would
                      // lead to e.g. doubles and chars both taking 1 memory address. This
                      // is a form of 'compressed' memory, with shrinking and stretching
                      // according to the type, when compared to C/C++. On the other hand
                      // the normal value of 4 means all fields take 4 memory addresses,
                      // as per the norm on a 32-bit machine.
                      //
                      // 1 is somewhat faster than 4, but dangerous.

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

var INVOKE_RUN = 1; // Whether we will call run(). Disable if you embed the generated
                    // code in your own, and will call run() yourself at the right time
var INIT_STACK = 0; // Whether to initialize memory on the stack to 0.
var INIT_HEAP = 0; // Whether to initialize memory anywhere other than the stack to 0.
var TOTAL_STACK = 5*1024*1024; // The total stack size. There is no way to enlarge the stack, so this
                               // value must be large enough for the program's requirements. If
                               // assertions are on, we will assert on not exceeding this, otherwise,
                               // it will fail silently.
var TOTAL_MEMORY = 10*1024*1024; // The total amount of memory to use. Using more memory than this will
                                 // cause us to expand the heap, which can be costly with typed arrays:
                                 // we need to copy the old heap into a new one in that case.
var FAST_MEMORY = 2*1024*1024; // The amount of memory to initialize to 0. This ensures it will be
                               // in a flat array. This only matters in non-typed array builds.

// Code embetterments
var MICRO_OPTS = 1; // Various micro-optimizations, like nativizing variables
var RELOOP = 0; // Recreate js native loops from llvm data
var USE_TYPED_ARRAYS = 2; // Use typed arrays for the heap
                          // 1 has two heaps, IHEAP (int32) and FHEAP (double),
                          // and addresses there are a match for normal addresses. This wastes memory but can be fast.
                          // 2 is a single heap, accessible through views as int8, int32, etc. This saves memory but
                          // has more overhead of pointer calculations. It also is limited to storing doubles as floats,
                          // simply because double stores are not necessarily 64-bit aligned, and we can only access
                          // 64-bit aligned values with a 64-bit typed array. Likewise int64s are stored as int32's,
                          // which is potentially very dangerous!
                          // TODO: require compiling with -malign-double, which does align doubles
var USE_FHEAP = 1; // Relevant in USE_TYPED_ARRAYS == 1. If this is disabled, only IHEAP will be used, and FHEAP
                   // not generated at all. This is useful if your code is 100% ints without floats or doubles
var I64_MODE = 1; // How to implement 64-bit integers:
                  // 0: As doubles. This will work up to about 53 bits.
                  // 1: As [low, high]. This will support all 64 bits for bit ops, etc. properly, but will still
                  //                    use doubles for addition etc., like mode 0. This mode is slower than
                  //                    mode 0, so its only benefit is proper support for 64 bit bitops.
                  // TODO: Full bignum support
var DOUBLE_MODE = 1; // How to load and store 64-bit doubles. Without typed arrays or in typed array mode 1,
                     // this doesn't matter - these values are just values like any other. In typed array mode 2,
                     // a potentialy risk is that doubles may be only 32-bit aligned. Forcing 64-bit alignment
                     // in Clang itself should be able to solve that, or as a workaround in DOUBLE_MODE 1 we
                     // will carefully load in parts, in a way that requires only 32-bit alignment. In DOUBLE_MODE
                     // 0 we will simply store and load doubles as 32-bit floats, so when they are stored/loaded
                     // they will truncate from 64 to 32 bits, and lose precision. This is faster, and might
                     // work for some code (but probably that code should just use floats and not doubles anyhow).
                     // Note that a downside of DOUBLE_MODE 1 is that we currently store the double in parts,
                     // then load it aligned, and that load-store will make JS engines alter it if it is being
                     // stored to a typed array for security reasons. That will 'fix' the number from being a
                     // NaN or an infinite number.
var EMULATE_UNALIGNED_ACCESSES = 0; // If set, the compiler will 'emulate' loads and stores that are not known to
                                    // be sufficiently aligned, by working on individual bytes. This can be
                                    // important in USE_TYPED_ARRAYS == 2, where unaligned accesses do not work,
                                    // specifically in the case where unsafe LLVM optimizations have generated possibly
                                    // unaligned code. (Without unsafe LLVM optimizations, there should be no
                                    // need for this option.)
                                    // Currently this only works for integers, not doubles and floats.

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
var CLOSURE_INLINE_PREVENTION_LINES = 50; // Functions of this number of lines or larger will have
                                          // code generated that tells the closure compiler not to
                                          // inline them. This is useful to prevent the generation of
                                          // overly large functions.
var CATCH_EXIT_CODE = 0; // If set, causes exit() to throw an exception object which is caught
                         // in a try..catch block and results in the exit status being
                         // returned from run(). If zero (the default), the program is just
                         // terminated with an error message.

// Generated code debugging options
var SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
                   // If equal to 2, done on a line-by-line basis according to
                   // SAFE_HEAP_LINES, checking only the specified lines.
                   // If equal to 3, checking all *but* the specified lines. Note
                   // that 3 is the option you usually want here.
var SAFE_HEAP_LOG = 0; // Log out all SAFE_HEAP operations

var LABEL_DEBUG = 0; // Print out labels and functions as we enter them
var EXCEPTION_DEBUG = 1; // Print out exceptions in emscriptened code
var LIBRARY_DEBUG = 0; // Print out when we enter a library call (library*.js)
var DISABLE_EXCEPTION_CATCHING = 0; // Disables generating code to actually catch exceptions. If the code you
                                    // are compiling does not actually rely on catching exceptions (but the
                                    // compiler generates code for it, maybe because of stdlibc++ stuff),
                                    // then this can make it much faster. If an exception actually happens,
                                    // it will not be caught and the program will halt (so this will not
                                    // introduce silent failures, which is good).
                                    // TODO: Make this also remove cxa_begin_catch etc., optimize relooper
                                    //       for it, etc. (perhaps do all of this as preprocessing on .ll?)
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

var PGO = 0; // Profile-guided optimization.
             // When run with the CHECK_* options, will not fail on errors. Instead, will
             // keep a record of which checks succeeded and which failed. On shutdown, will
             // print out that information. This is useful for knowing which lines need
             // checking enabled and which do not, that is, this is a way to automate the
             // generation of line data for CORRECT_*_LINES options.
             // All CORRECT_* options default to 1 with PGO builds.
             // See https://github.com/kripken/emscripten/wiki/Optimizing-Code for more info

var PROFILE = 0; // Enables runtime profiling. See test_profiling for a usage example.

var EXPORTED_FUNCTIONS = ['_main']; // Functions that are explicitly exported, so they are guaranteed to
                                    // be accessible outside of the generated code.

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

// A set of defines, for example generated from your header files. This
// lets the emscripten libc (library.js) see the right values.
// The default value here has been generated from system/include. If you
// modify those files, or use different headers, you will need to override
// this.
var C_DEFINES = {"SI_MESGQ":"5","M_SQRTPI":"1.77245385091","_NL_MESSAGES_CODESET":"86","SIGRTMIN":"27","math_errhandling":"1","M_LOG10E":"0.434294481903","_S_IFMT":"0170000","_CS_V7_ENV":"20","USHRT_MAX":"65535","_SC_XOPEN_LEGACY":"98","_SC_XOPEN_VERSION":"106","F_UNLCK":"3","_SC_BC_DIM_MAX":"58","S_IFDIR":"0040000","_SC_BARRIERS":"56","_IFDIR":"0040000","_IFLNK":"0120000","__long_double_t":"<type 'long'>","DEFFILEMODE":"0000400","_FCREAT":"512","O_CREAT":"512","SHRT_MAX":"32767","_SC_NPROCESSORS_CONF":"9","_XOPEN_ENH_I18N":"1","F_DUPFD_CLOEXEC":"14","_CS_POSIX_V6_LP64_OFF64_LIBS":"11","_POSIX_SHARED_MEMORY_OBJECTS":"200112","ABDAY_7":"20","ABDAY_6":"19","ABDAY_5":"18","ABDAY_4":"17","ABDAY_3":"16","M_PI":"3.14159265359","ABDAY_1":"14","_PC_REC_MIN_XFER_SIZE":"18","_SC_V6_ILP32_OFFBIG":"93","SIGSTOP":"17","_M_LN2":"0.69314718056","F_UNLKSYS":"4","PTHREAD_CREATE_JOINABLE":"1","M_PI_2":"1.57079632679","_SC_MEMLOCK":"24","M_PI_4":"0.785398163397","FEXLOCK":"256","_FNDELAY":"16384","SIGEV_NONE":"1","SIGWINCH":"28","UTIME_NOW":"-2","_SC_THREADS":"42","__INT_MAX__":"2147483647","_XBS5_LP64_OFF64":"-1","_CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS":"6","___int_least32_t_defined":"1","_POSIX_MAPPED_FILES":"200112","M_E":"2.71828182846","SIGTRAP":"5","_SC_SS_REPL_MAX":"78","POLLIN":"1","_SC_THREAD_KEYS_MAX":"38","O_RDWR":"2","__LARGE64_FILES":"1","_POSIX_V6_LP64_OFF64":"-1","_SC_2_PBS":"113","FNOCTTY":"32768","_SC_TRACE_INHERIT":"86","PTHREAD_PRIO_NONE":"0","_SC_REGEXP":"72","_CS_POSIX_V6_LP64_OFF64_CFLAGS":"9","_SC_DELAYTIMER_MAX":"37","_POSIX_RAW_SOCKETS":"200112","___int64_t_defined":"1","S_IFREG":"0100000","SIGCLD":"20","F_GETLK64":"20","_IFCHR":"0020000","S_IRWXG":"0000040","S_IFMT":"0170000","RADIXCHAR":"50","_S_IEXEC":"0000100","_SC_XOPEN_CRYPT":"96","M_LN10":"2.30258509299","S_IRWXU":"0000400","OPTIONAL_ARG":"2","_PC_CHOWN_RESTRICTED":"6","CRNCYSTR":"56","SCHAR_MAX":"127","S_BLKSIZE":"1024","_SC_CLK_TCK":"2","AM_STR":"5","__BUFSIZ__":"16","ALT_DIGITS":"49","_NL_CTYPE_MB_CUR_MAX":"85","_REENT_SIGNAL_SIZE":"24","F_RSETLKW":"13","___int16_t_defined":"1","SIGXCPU":"24","_SC_MQ_PRIO_MAX":"14","_FTRUNC":"1024","MALLOC_ALIGNMENT":"16","PTHREAD_CREATE_DETACHED":"0","_POSIX2_VERSION":"200112","_O_CREAT":"512","PM_STR":"6","_PC_POSIX_SECURITY":"91","_SC_2_LOCALEDEF":"112","_SC_STREAM_MAX":"100","_CS_POSIX_V7_ILP32_OFF32_LIBS":"3","_POSIX2_C_BIND":"200112","_POSIX_VERSION":"200112","S_IFIFO":"0010000","SCHED_FIFO":"1","M_LN2LO":"1.90821492927e-10","MON_10":"30","_CS_XBS5_ILP32_OFF32_LIBS":"3","O_SYNC":"8192","_CS_POSIX_V6_ILP32_OFFBIG_LIBS":"7","YESEXPR":"52","_PC_PATH_MAX":"4","_SC_SPORADIC_SERVER":"77","_POSIX_SYNCHRONIZED_IO":"200112","SIGXFSZ":"25","_SC_NPROCESSORS_ONLN":"10","_CS_POSIX_V6_LPBIG_OFFBIG_LIBS":"15","_PC_MAX_INPUT":"2","F_TLOCK":"2","REQUIRED_ARG":"1","_SC_VERSION":"7","HUGE_VAL":"inf","AT_EACCESS":"1","ABMON_3":"35","ABMON_2":"34","ABMON_1":"33","_SC_THREAD_ROBUST_PRIO_PROTECT":"123","ABMON_7":"39","ABMON_6":"38","ABMON_5":"37","ABMON_4":"36","ABMON_9":"41","ABMON_8":"40","UNDERFLOW":"4","_SC_AIO_MAX":"35","ERA":"45","_CS_XBS5_ILP32_OFFBIG_LIBS":"7","S_IXUSR":"0000100","_SC_THREAD_PRIO_INHERIT":"46","M_2_PI":"0.636619772368","_O_RDWR":"2","_PC_2_SYMLINKS":"13","SIG_BLOCK":"1","_O_WRONLY":"1","_CS_XBS5_LP64_OFF64_CFLAGS":"9","MON_2":"22","MON_3":"23","_POSIX_PRIORITY_SCHEDULING":"200112","MON_1":"21","MON_6":"26","MON_7":"27","MON_4":"24","MON_5":"25","_SC_SPAWN":"75","MON_8":"28","MON_9":"29","_CS_POSIX_V6_ILP32_OFF32_LDFLAGS":"2","_SC_TRACE_EVENT_NAME_MAX":"85","SA_SIGINFO":"2","_FBINARY":"65536","PTHREAD_PRIO_PROTECT":"2","POLLERR":"8","SIGVTALRM":"26","O_BINARY":"65536","_REENT_EMERGENCY_SIZE":"25","S_IEXEC":"0000100","_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS":"13","SIGEV_THREAD":"3","ITIMER_VIRTUAL":"1","_SC_TRACE_SYS_MAX":"89","_POSIX_NO_TRUNC":"1","_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS":"17","F_SETFL":"4","_CS_POSIX_V7_THREADS_CFLAGS":"18","_SC_AIO_PRIO_DELTA_MAX":"36","_POSIX2_C_DEV":"200112","_SC_MONOTONIC_CLOCK":"69","_POSIX_THREAD_SPORADIC_SERVER":"1","_FNOINHERIT":"262144","_SC_XOPEN_ENH_I18N":"97","SIGPROF":"27","F_SETLKW64":"22","_O_APPEND":"8","_FDEFER":"32","CLOCK_DISALLOWED":"0","_SC_MEMORY_PROTECTION":"26","LONG_MAX":"2147483647","no_argument":"0","F_OK":"0","FAPPEND":"8","FREAD":"1","_SC_SPIN_LOCKS":"76","M_LN2HI":"0.693147180369","_LIBC_LIMITS_H_":"1","S_IFSOCK":"0140000","_POSIX_DEVCTL_DIRECTION":"1","ABDAY_2":"15","_PC_LINK_MAX":"0","_POSIX_THREAD_PRIO_PROTECT":"1","NOEXPR":"53","FEXCL":"2048","_SC_FSYNC":"22","_SC_GETGR_R_SIZE_MAX":"50","_POSIX_THREAD_PROCESS_SHARED":"200112","_ATEXIT_SIZE":"32","_SC_TRACE_NAME_MAX":"88","_SC_BC_BASE_MAX":"57","_LIMITS_H":"1","PTHREAD_STACK_MIN":"200","O_APPEND":"8","_CS_XBS5_LP64_OFF64_LINTFLAGS":"12","_SC_XOPEN_STREAMS":"104","_SC_GETPW_R_SIZE_MAX":"51","_POSIX_ASYNCHRONOUS_IO":"1","UCHAR_MAX":"255","_SC_PAGE_SIZE":"8","_SC_XBS5_ILP32_OFFBIG":"93","S_IFBLK":"0060000","_S_IFIFO":"0010000","T_FMT_AMPM":"4","_POSIX_SEMAPHORES":"200112","NBBY":"8","SIGEMT":"7","_POSIX_FSYNC":"200112","F_SETLKW":"9","_MB_EXTENDED_CHARSETS_WINDOWS":"1","SIGALRM":"14","___int32_t_defined":"1","_POSIX2_UPE":"200112","_SC_OPEN_MAX":"4","CHAR_BIT":"8","_N_LISTS":"30","_SC_2_FORT_RUN":"111","PTHREAD_MUTEX_DEFAULT":"3","_POSIX_REGEXP":"1","_SC_RE_DUP_MAX":"73","PTHREAD_PROCESS_PRIVATE":"0","_S_IFREG":"0100000","_SC_THREAD_THREADS_MAX":"40","_SC_THREAD_PRIO_PROTECT":"47","_SC_2_PBS_CHECKPOINT":"115","M_1_PI":"0.318309886184","_PC_POSIX_PERMISSIONS":"90","_SC_TIMERS":"33","MON_11":"31","MON_12":"32","CLOCK_DISABLED":"0","_SC_XBS5_LPBIG_OFFBIG":"95","_POSIX_SPIN_LOCKS":"200112","_FREAD":"1","_SC_SHARED_MEMORY_OBJECTS":"199","F_RDLCK":"1","F_GETFD":"1","AT_SYMLINK_NOFOLLOW":"2","_PC_ALLOC_SIZE_MIN":"15","_POSIX_C_SOURCE":"2","_SC_READER_WRITER_LOCKS":"71","SI_USER":"1","_SC_MEMLOCK_RANGE":"25","_SC_PRIORITY_SCHEDULING":"101","optional_argument":"2","T_FMT":"3","_PC_VDISABLE":"8","THOUSEP":"51","O_NOINHERIT":"262144","PTHREAD_PROCESS_SHARED":"1","_SC_TRACE_EVENT_FILTER":"84","ERA_T_FMT":"48","_SC_THREAD_ATTR_STACKADDR":"43","_SC_LOGIN_NAME_MAX":"52","M_LOG2E":"1.44269504089","ITIMER_PROF":"2","_SC_2_C_BIND":"108","FNONBIO":"16384","_PC_NO_TRUNC":"7","F_RSETLK":"11","_SC_V7_ILP32_OFF32":"92","_FAPPEND":"8","PTHREAD_EXPLICIT_SCHED":"2","_FNBIO":"4096","F_CNVT":"12","_SC_SHELL":"74","_SC_V6_LP64_OFF64":"94","SCHED_OTHER":"0","_CS_GNU_LIBC_VERSION":"42","_SC_SEM_VALUE_MAX":"17","S_ENFMT":"0002000","_SC_MQ_OPEN_MAX":"13","_POSIX_ADVISORY_INFO":"200112","SIGABRT":"6","_CS_POSIX_V7_ILP32_OFF32_CFLAGS":"1","_CS_XBS5_ILP32_OFF32_CFLAGS":"1","_MB_EXTENDED_CHARSETS_ISO":"1","_SC_HOST_NAME_MAX":"65","_SC_THREAD_STACK_MIN":"39","_SC_TIMEOUTS":"82","POLLOUT":"2","_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS":"16","_SC_CHILD_MAX":"1","__RAND_MAX":"2147483647","_POSIX_THREAD_ATTR_STACKADDR":"1","SIGFPE":"8","NL_ARGMAX":"32","_SC_2_PBS_MESSAGE":"117","TIMER_ABSTIME":"4","_NL_CTYPE_CODESET_NAME":"0","_SC_2_C_DEV":"109","_SC_TIMER_MAX":"19","S_IXOTH":"0000001","FP_ZERO":"2","SING":"2","M_INVLN2":"1.44269504089","M_TWOPI":"3.14159265359","_PC_REC_XFER_ALIGN":"19","_NL_TIME_DATE_FMT":"84","_SC_REALTIME_SIGNALS":"29","_POSIX2_RE_DUP_MAX":"255","CLOCKS_PER_SEC":"1000","_READ_WRITE_RETURN_TYPE":"<type 'int'>","ERA_D_T_FMT":"47","SIG_UNBLOCK":"2","_CS_XBS5_ILP32_OFFBIG_LDFLAGS":"6","_FSHLOCK":"128","CLK_TCK":"1000","D_FMT":"2","_POSIX_SPAWN":"1","_XBS5_ILP32_OFF32":"-1","_SC_THREAD_PRIO_CEILING":"47","SCHED_SPORADIC":"4","_PC_ASYNC_IO":"9","SI_TIMER":"3","DAY_2":"8","DAY_3":"9","DAY_1":"7","DAY_6":"12","DAY_7":"13","DAY_4":"10","DAY_5":"11","F_GETFL":"3","AT_REMOVEDIR":"8","PATH_MAX":"4096","_POSIX_TIMEOUTS":"1","_SC_MAPPED_FILES":"23","_SC_NGROUPS_MAX":"3","_FSYNC":"8192","MATH_ERRNO":"1","_POSIX_SAVED_IDS":"1","_SC_SEMAPHORES":"30","__FILENAME_MAX__":"255","SIGTSTP":"18","F_ULOCK":"0","__LONG_MAX__":"2147483647","F_WRLCK":"2","_POSIX_JOB_CONTROL":"1","FLT_EVAL_METHOD":"0","_XOPEN_SHM":"1","_POSIX_CHOWN_RESTRICTED":"1","F_SETLK64":"21","_SC_TRACE_LOG":"87","SIGILL":"4","_FNONBLOCK":"16384","_POSIX_TIMERS":"1","FNDELAY":"16384","FD_CLOEXEC":"1","_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS":"14","_POSIX_THREAD_CPUTIME":"1","F_LOCK":"1","_FLOAT_ARG":"<type 'float'>","_CS_XBS5_ILP32_OFF32_LINTFLAGS":"4","_SC_THREAD_DESTRUCTOR_ITERATIONS":"53","M_LN2":"0.69314718056","UINT_MAX":"2147483647","SIG_SETMASK":"0","_SC_BC_STRING_MAX":"60","_CS_XBS5_LPBIG_OFFBIG_LIBS":"15","_SC_XOPEN_UUCP":"124","_SC_2_SW_DEV":"119","FDEFER":"32","FP_NAN":"0","F_SETOWN":"6","CHAR_MIN":"-128","PTHREAD_SCOPE_SYSTEM":"1","_POSIX_V6_LPBIG_OFFBIG":"-1","_S_IWRITE":"0000200","_S_IFDIR":"0040000","_SC_ARG_MAX":"0","_SC_THREAD_PRIORITY_SCHEDULING":"45","F_GETLK":"7","SIGTTIN":"21","_CS_POSIX_V7_WIDTH_RESTRICTED_ENVS":"17","_POSIX_SPORADIC_SERVER":"1","_SC_THREAD_CPUTIME":"80","_POSIX_V6_ILP32_OFF32":"-1","_CS_POSIX_V7_ILP32_OFFBIG_LIBS":"7","_CS_POSIX_V6_ILP32_OFF32_LIBS":"3","_SC_SYNCHRONIZED_IO":"32","_UNIX98_THREAD_MUTEX_ATTRIBUTES":"1","_POSIX_REALTIME_SIGNALS":"200112","__SIGLASTNOTRT":"31","ERA_D_FMT":"46","MB_LEN_MAX":"1","_FASYNC":"64","_CS_POSIX_V6_ILP32_OFF32_CFLAGS":"1","NOSTR":"55","_POSIX_MONOTONIC_CLOCK":"200112","SIGPOLL":"23","S_ISGID":"0002000","FP_INFINITE":"1","ULONG_MAX":"2147483647","__SIGFIRSTNOTRT":"1","AT_SYMLINK_FOLLOW":"4","FSYNC":"8192","__USE_XOPEN2K":"1","_CS_XBS5_ILP32_OFFBIG_CFLAGS":"5","_IFSOCK":"0140000","_IFIFO":"0010000","ARG_MAX":"4096","SIGPIPE":"13","_O_EXCL":"2048","O_TRUNC":"1024","O_TEXT":"131072","_POSIX_THREAD_PRIO_INHERIT":"1","_XBS5_ILP32_OFFBIG":"1","STDERR_FILENO":"2","_CS_XBS5_LPBIG_OFFBIG_CFLAGS":"13","PLOSS":"6","S_IRWXO":"0000004","_SC_V7_LP64_OFF64":"94","_O_NOINHERIT":"262144","D_MD_ORDER":"57","_IFMT":"0170000","_SC_SYMLOOP_MAX":"79","_SC_XOPEN_UNIX":"105","M_IVLN10":"0.434294481903","ALLPERMS":"0004000","F_SETFD":"2","SIGUSR1":"30","___int8_t_defined":"1","SIGKILL":"9","_CS_POSIX_V7_ILP32_OFFBIG_CFLAGS":"5","PTHREAD_MUTEX_RECURSIVE":"1","SIGSEGV":"11","M_LOG2_E":"0.69314718056","FWRITE":"2","_FEXCL":"2048","SIGINT":"2","H8300":"1","_POSIX_MEMORY_PROTECTION":"200112","FP_ILOGBNAN":"2147483647","_SC_V7_LPBIG_OFFBIG":"95","_SC_CLOCK_SELECTION":"61","_PC_PRIO_IO":"10","M_2_SQRTPI":"1.1283791671","S_IROTH":"0000004","_SC_MESSAGE_PASSING":"27","_SC_V6_LPBIG_OFFBIG":"95","_SC_EXPR_NEST_MAX":"64","_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS":"5","S_ISVTX":"0001000","UTIME_OMIT":"-1","_CS_POSIX_V7_THREADS_LDFLAGS":"19","_SC_PAGESIZE":"8","_PC_REC_MAX_XFER_SIZE":"17","SIGIOT":"6","FASYNC":"64","_SC_V7_ILP32_OFFBIG":"93","_RAND48_MULT_0":"58989","_RAND48_MULT_1":"57068","_RAND48_MULT_2":"5","_CS_POSIX_V6_LP64_OFF64_LDFLAGS":"10","PTHREAD_MUTEX_NORMAL":"0","_O_TRUNC":"1024","W_OK":"2","O_NONBLOCK":"16384","R_OK":"4","_IFBLK":"0060000","FTRUNC":"1024","_SC_XBS5_LP64_OFF64":"94","STDIN_FILENO":"0","_CS_POSIX_V7_LPBIG_OFFBIG_LIBS":"15","_SC_SIGQUEUE_MAX":"18","M_3PI_4":"2.35619449019","_POSIX_MESSAGE_PASSING":"200112","S_ISUID":"0004000","SIGLOST":"29","_POSIX_THREAD_PRIORITY_SCHEDULING":"200112","_SC_TZNAME_MAX":"20","_O_RAW":"65536","_CS_PATH":"0","_POSIX_BARRIERS":"200112","SEEK_SET":"0","_POSIX_THREAD_SAFE_FUNCTIONS":"200112","S_IREAD":"0000400","_LONG_LONG_TYPE":"<type 'long'>","___int_least8_t_defined":"1","INT_MAX":"2147483647","_POSIX_V6_ILP32_OFFBIG":"1","_SC_PHYS_PAGES":"11","_PC_MAX_CANON":"1","_CS_XBS5_LPBIG_OFFBIG_LDFLAGS":"14","_SC_THREAD_SAFE_FUNCTIONS":"49","SIGRTMAX":"31","S_IXGRP":"0000010","_XBS5_LPBIG_OFFBIG":"-1","_PC_NAME_MAX":"3","O_EXCL":"2048","_SC_XOPEN_SHM":"103","S_IWGRP":"0000020","_SC_TRACE_USER_EVENT_MAX":"90","M_SQRT1_2":"0.707106781187","_SC_AVPHYS_PAGES":"12","_SC_RAW_SOCKETS":"70","O_RDONLY":"0","_DATE_FMT":"84","_SC_RTSIG_MAX":"15","_POSIX_DEVICE_CONTROL":"1","CLOCK_ENABLED":"1","_NULL":"0","_SC_PRIORITIZED_IO":"28","_O_TEXT":"131072","SIGBUS":"10","CODESET":"0","CHAR_MAX":"127","SIGSYS":"12","_PC_REC_INCR_XFER_SIZE":"16","S_IRUSR":"0000400","_PC_FILESIZEBITS":"12","_SC_XBS5_ILP32_OFF32":"92","SIGURG":"16","_POSIX_THREAD_ATTR_STACKSIZE":"200112","_CS_POSIX_V7_LP64_OFF64_LIBS":"11","_CS_GNU_LIBPTHREAD_VERSION":"43","_REENT_ASCTIME_SIZE":"26","_SC_2_PBS_LOCATE":"116","_SC_V6_ILP32_OFF32":"92","SIGCHLD":"20","SHRT_MIN":"-32768","PTHREAD_MUTEX_ERRORCHECK":"2","_PC_SYNC_IO":"11","FP_NORMAL":"4","_SC_2_UPE":"120","_SC_SEM_NSEMS_MAX":"16","_SC_IOV_MAX":"66","S_IRGRP":"0000040","YESSTR":"54","S_IFCHR":"0020000","_POSIX_MEMLOCK":"1","_SC_TRACE":"83","_POSIX_INTERRUPT_CONTROL":"1","_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS":"6","PTHREAD_SCOPE_PROCESS":"0","_CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS":"14","TLOSS":"5","_TIME_T_":"<type 'long'>","DOMAIN":"1","_POINTER_INT":"<type 'long'>","O_NOCTTY":"32768","PTHREAD_PRIO_INHERIT":"1","_SC_THREAD_SPORADIC_SERVER":"81","O_ACCMODE":"3","_SC_LINE_MAX":"68","D_T_FMT":"1","_RAND48_SEED_1":"43981","_RAND48_SEED_0":"13070","_RAND48_SEED_2":"4660","FOPEN":"-1","F_RGETLK":"10","F_DUPFD":"0","_S_IFCHR":"0020000","_POSIX_IPV6":"200112","_XOPEN_VERSION":"600","_CS_XBS5_LP64_OFF64_LDFLAGS":"10","_FNOCTTY":"32768","_IFREG":"0100000","FP_SUBNORMAL":"3","SA_NOCLDSTOP":"1","NSIG":"32","S_IWOTH":"0000002","SEEK_END":"2","S_IWUSR":"0000200","SI_ASYNCIO":"4","required_argument":"1","_FWRITE":"2","SCHED_RR":"2","_SC_2_FORT_DEV":"110","_POSIX2_CHAR_TERM":"200112","F_SETLK":"8","SIGQUIT":"3","_SC_ATEXIT_MAX":"55","_POSIX_MEMLOCK_RANGE":"200112","_SC_SAVED_IDS":"6","PTHREAD_INHERIT_SCHED":"1","SEEK_CUR":"1","S_IFLNK":"0120000","_S_IREAD":"0000400","_FOPEN":"-1","OVERFLOW":"3","_POSIX_SHELL":"1","FMARK":"16","POLLNVAL":"4","_SC_2_PBS_TRACK":"118","_POSIX_CPUTIME":"1","_SC_THREAD_PROCESS_SHARED":"48","_SC_JOB_CONTROL":"5","_O_RDONLY":"0","FNBIO":"4096","O_CLOEXEC":"262144","_CS_XBS5_LP64_OFF64_LIBS":"11","_PC_TIMESTAMP_RESOLUTION":"20","_POSIX_READER_WRITER_LOCKS":"200112","ACCESSPERMS":"0000400","_POSIX_PRIORITIZED_IO":"1","_SC_IPV6":"67","_CS_XBS5_ILP32_OFFBIG_LINTFLAGS":"8","_SC_ADVISORY_INFO":"54","SCHAR_MIN":"-128","_SC_XOPEN_REALTIME_THREADS":"102","SIGEV_SIGNAL":"2","O_WRONLY":"1","_PC_SYMLINK_MAX":"14","_SC_THREAD_ROBUST_PRIO_INHERIT":"122","X_OK":"1","_CS_XBS5_ILP32_OFF32_LDFLAGS":"2","SIGTERM":"15","_SC_COLL_WEIGHTS_MAX":"62","_CS_POSIX_V7_ILP32_OFF32_LDFLAGS":"2","NO_ARG":"0","_CS_POSIX_V7_LP64_OFF64_CFLAGS":"9","S_IWRITE":"0000200","_FEXLOCK":"256","_SC_XOPEN_REALTIME":"99","SIGPWR":"19","_PC_PIPE_BUF":"5","SIGHUP":"1","F_GETOWN":"5","CLOCK_ALLOWED":"1","SIGUSR2":"31","_SC_2_PBS_ACCOUNTING":"114","F_TEST":"3","ITIMER_REAL":"0","_CLOCKS_PER_SEC_":"1000","SIGCONT":"19","FSHLOCK":"128","STDOUT_FILENO":"1","_SC_THREAD_ATTR_STACKSIZE":"44","SIGIO":"23","_SC_CPUTIME":"63","_SC_2_VERSION":"121","_O_BINARY":"65536","___int_least16_t_defined":"1","_FTEXT":"131072","_POSIX2_SW_DEV":"200112","_LARGEFILE64_SOURCE":"1","_XOPEN_CRYPT":"1","FD_SETSIZE":"64","_FMARK":"16","_SC_TYPED_MEMORY_OBJECTS":"91","_SC_ASYNCHRONOUS_IO":"21","_SC_2_CHAR_TERM":"107","_SC_AIO_LISTIO_MAX":"34","FCREAT":"512","_RAND48_ADD":"11","MAXPATHLEN":"1024","_SC_BC_SCALE_MAX":"59","MATH_ERREXCEPT":"2","_CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS":"13","M_SQRT3":"1.73205080757","M_SQRT2":"1.41421356237","_POSIX_THREADS":"200112","ABMON_12":"44","ABMON_11":"43","ABMON_10":"42","SI_QUEUE":"2","AT_FDCWD":"-2","SIGTTOU":"22","_CS_POSIX_V7_LP64_OFF64_LDFLAGS":"10","_SC_TTY_NAME_MAX":"41"};

var SHOW_LABELS = 0; // Show labels in the generated code

var BUILD_AS_SHARED_LIB = 0; // Whether to build the code as a shared library, which
                             // must be loaded dynamically using dlopen().
                             // 0 here means this is not a shared lib: It is a main file.
                             // 1 means this is a normal shared lib.
                             // 2 means this is a shared lib that will be linked at runtime,
                             //   which means it will insert its functions into
                             //   the global namespace. See STATIC_LIBS_TO_LOAD.
var RUNTIME_LINKED_LIBS = []; // If this is a main file (BUILD_AS_SHARED_LIB == 0), then
                              // we will link these at runtime. They must have been built with
                              // BUILD_AS_SHARED_LIB == 2.
                              // NOTE: LLVM optimizations run separately on the main file and
                              //       linked libraries can break things.

var RUNTIME_TYPE_INFO = 0; // Whether to expose type info to the script at run time. This
                           // increases the size of the generated script, but allows you
                           // to more easily perform operations from handwritten JS on
                           // objects with structures etc.

var FAKE_X86_FP80 = 0; // Replaces x86_fp80 with double. This loses precision. It is better,
                       // if you can, to get the original source code to build without x86_fp80
                       // (which is nonportable anyhow).

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

