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
C_DEFINES = {"S_IWRITE":"0000200", "_POSIX_CPUTIME":"1", "O_RDONLY":"0", "_POSIX_C_SOURCE":"2", "__FILENAME_MAX__":"255", "DEFFILEMODE":"0000400", "PTHREAD_PRIO_NONE":"0", "_S_IFMT":"0170000", "_O_TEXT":"131072", "_FNOINHERIT":"262144", "F_WRLCK":"2", "_POSIX_JOB_CONTROL":"1", "_FASYNC":"64", "__BUFSIZ__":"16", "S_IRUSR":"0000400", "F_UNLCK":"3", "_FDEFER":"32", "_O_EXCL":"2048", "AT_SYMLINK_NOFOLLOW":"2", "S_IFDIR":"0040000", "_POSIX_THREAD_ATTR_STACKSIZE":"200112", "_IFDIR":"0040000", "_POSIX_TIMERS":"1", "_IFLNK":"0120000", "FNDELAY":"16384", "_POSIX2_C_DEV":"200112", "_FCREAT":"512", "_POSIX_THREAD_CPUTIME":"1", "FAPPEND":"8", "O_CREAT":"512", "_POSIX_ADVISORY_INFO":"200112", "_MB_EXTENDED_CHARSETS_WINDOWS":"1", "_POSIX_V6_LPBIG_OFFBIG":"-1", "PTHREAD_MUTEX_ERRORCHECK":"2", "_XBS5_ILP32_OFF32":"-1", "_POSIX_THREAD_SPORADIC_SERVER":"1", "FD_CLOEXEC":"1", "F_DUPFD_CLOEXEC":"14", "_XBS5_LP64_OFF64":"-1", "_POSIX_SHARED_MEMORY_OBJECTS":"200112", "_POSIX_MEMORY_PROTECTION":"200112", "_POSIX_DEVCTL_DIRECTION":"1", "S_IRGRP":"0000040", "FDEFER":"32", "S_IFCHR":"0020000", "F_SETOWN":"6", "_POSIX_THREAD_PRIO_PROTECT":"1", "___int_least16_t_defined":"1", "PTHREAD_SCOPE_SYSTEM":"1", "S_ISGID":"0002000", "_POSIX_INTERRUPT_CONTROL":"1", "FEXCL":"2048", "PTHREAD_SCOPE_PROCESS":"0", "_S_IFDIR":"0040000", "F_RSETLK":"11", "_POSIX_READER_WRITER_LOCKS":"200112", "F_UNLKSYS":"4", "__RAND_MAX":"2147483647", "PTHREAD_CREATE_JOINABLE":"1", "_S_IFIFO":"0010000", "FEXLOCK":"256", "_FNDELAY":"16384", "_POSIX_SPORADIC_SERVER":"1", "_MB_EXTENDED_CHARSETS_ISO":"1", "PTHREAD_STACK_MIN":"200", "O_APPEND":"8", "_POSIX_DEVICE_CONTROL":"1", "_POSIX_V6_ILP32_OFF32":"-1", "_POSIX_SPIN_LOCKS":"200112", "O_NOCTTY":"32768", "UTIME_NOW":"-2", "O_ACCMODE":"3", "_FSHLOCK":"128", "_POSIX_REGEXP":"1", "___int_least32_t_defined":"1", "_UNIX98_THREAD_MUTEX_ATTRIBUTES":"1", "_FTRUNC":"1024", "_POSIX_BARRIERS":"200112", "_POSIX_FSYNC":"200112", "F_SETLKW":"9", "_POSIX_RAW_SOCKETS":"200112", "O_RDWR":"2", "FOPEN":"-1", "F_RGETLK":"10", "F_DUPFD":"0", "__LARGE64_FILES":"1", "_S_IFCHR":"0020000", "_XOPEN_SHM":"1", "AT_SYMLINK_FOLLOW":"4", "_S_IWRITE":"0000200", "FSYNC":"8192", "_POSIX_CHOWN_RESTRICTED":"1", "FNOCTTY":"32768", "__USE_XOPEN2K":"1", "PTHREAD_MUTEX_DEFAULT":"3", "_IFSOCK":"0140000", "_FNOCTTY":"32768", "_IFIFO":"0010000", "PTHREAD_PROCESS_PRIVATE":"0", "_IFREG":"0100000", "S_BLKSIZE":"1024", "_POSIX_VERSION":"200112", "O_TRUNC":"1024", "_O_CREAT":"512", "___int64_t_defined":"1", "S_IFREG":"0100000", "O_TEXT":"131072", "_POSIX_THREAD_PRIO_INHERIT":"1", "F_GETLK":"7", "S_IWOTH":"0000002", "F_GETFD":"1", "_XBS5_ILP32_OFFBIG":"1", "S_IFMT":"0170000", "_O_RDWR":"2", "_FREAD":"1", "F_GETFL":"3", "_FWRITE":"2", "F_RDLCK":"1", "S_IRWXO":"0000004", "SCHED_SPORADIC":"4", "S_IRWXU":"0000400", "_O_NOINHERIT":"262144", "_POSIX_THREAD_SAFE_FUNCTIONS":"200112", "_IFMT":"0170000", "_POSIX2_CHAR_TERM":"200112", "F_SETLK":"8", "S_IWUSR":"0000200", "FNBIO":"4096", "ALLPERMS":"0004000", "_POSIX_V6_LP64_OFF64":"-1", "_POSIX_MEMLOCK_RANGE":"200112", "PTHREAD_INHERIT_SCHED":"1", "F_RSETLKW":"13", "F_SETFD":"2", "S_IFLNK":"0120000", "___int16_t_defined":"1", "_S_IREAD":"0000400", "_FNONBLOCK":"16384", "F_SETFL":"4", "_POSIX_SHELL":"1", "FMARK":"16", "___int8_t_defined":"1", "_POSIX2_UPE":"200112", "MALLOC_ALIGNMENT":"16", "PTHREAD_MUTEX_RECURSIVE":"1", "PTHREAD_CREATE_DETACHED":"0", "_POSIX2_VERSION":"200112", "FWRITE":"2", "FREAD":"1", "O_CLOEXEC":"262144", "H8300":"1", "ITIMER_PROF":"2", "FNONBIO":"16384", "_FNBIO":"4096", "O_WRONLY":"1", "_POSIX_MONOTONIC_CLOCK":"200112", "O_NOINHERIT":"262144", "ACCESSPERMS":"0000400", "PTHREAD_EXPLICIT_SCHED":"2", "_POSIX_PRIORITIZED_IO":"1", "PATH_MAX":"4096", "F_CNVT":"12", "_POSIX_THREAD_PROCESS_SHARED":"200112", "_POSIX2_C_BIND":"200112", "SCHED_OTHER":"0", "FCREAT":"512", "_POSIX_SPAWN":"1", "S_IFIFO":"0010000", "_POSIX_MEMLOCK":"1", "S_ISVTX":"0001000", "SCHED_FIFO":"1", "S_IXUSR":"0000100", "UTIME_OMIT":"-1", "S_ENFMT":"0002000", "O_SYNC":"8192", "S_IRWXG":"0000040", "PTHREAD_PROCESS_SHARED":"1", "_IFBLK":"0060000", "_FEXLOCK":"256", "_XOPEN_VERSION":"600", "_POSIX_SYNCHRONIZED_IO":"200112", "S_IROTH":"0000004", "_POSIX_MAPPED_FILES":"200112", "FASYNC":"64", "ARG_MAX":"4096", "_POSIX_NO_TRUNC":"1", "_XOPEN_ENH_I18N":"1", "AT_EACCESS":"1", "F_GETOWN":"5", "PTHREAD_MUTEX_NORMAL":"0", "_FEXCL":"2048", "_O_TRUNC":"1024", "ITIMER_REAL":"0", "_S_IEXEC":"0000100", "_POSIX_ASYNCHRONOUS_IO":"1", "PTHREAD_PRIO_INHERIT":"1", "FTRUNC":"1024", "S_IXOTH":"0000001", "O_NONBLOCK":"16384", "FSHLOCK":"128", "___int32_t_defined":"1", "_POSIX2_RE_DUP_MAX":"255", "_O_APPEND":"8", "_FLOAT_ARG":"<type 'float'>", "_POSIX_MESSAGE_PASSING":"200112", "S_ISUID":"0004000", "_READ_WRITE_RETURN_TYPE":"<type 'int'>", "_O_WRONLY":"1", "_POSIX_THREAD_PRIORITY_SCHEDULING":"200112", "_S_IFREG":"0100000", "_O_BINARY":"65536", "_XOPEN_CRYPT":"1", "_O_RAW":"65536", "_FTEXT":"131072", "_POSIX2_SW_DEV":"200112", "_POSIX_PRIORITY_SCHEDULING":"200112", "_LARGEFILE64_SOURCE":"1", "_POINTER_INT":"<type 'long'>", "_POSIX_SEMAPHORES":"200112", "_IFCHR":"0020000", "_FMARK":"16", "_POSIX_IPV6":"200112", "S_IREAD":"0000400", "_LONG_LONG_TYPE":"<type 'long'>", "S_IFSOCK":"0140000", "___int_least8_t_defined":"1", "_POSIX_V6_ILP32_OFFBIG":"1", "_FBINARY":"65536", "_FOPEN":"-1", "_O_RDONLY":"0", "O_BINARY":"65536", "SCHED_RR":"2", "_FAPPEND":"8", "ITIMER_VIRTUAL":"1", "S_IXGRP":"0000010", "AT_REMOVEDIR":"8", "_XBS5_LPBIG_OFFBIG":"-1", "O_EXCL":"2048", "S_IFBLK":"0060000", "_POSIX_TIMEOUTS":"1", "_POSIX_THREAD_ATTR_STACKADDR":"1", "S_IWGRP":"0000020", "_POSIX_THREADS":"200112", "S_IEXEC":"0000100", "_POSIX_REALTIME_SIGNALS":"200112", "PTHREAD_PRIO_PROTECT":"2", "AT_FDCWD":"-2", "_FSYNC":"8192", "_POSIX_SAVED_IDS":"1",
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

