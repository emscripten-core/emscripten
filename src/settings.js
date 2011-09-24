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

