// Tuning
QUANTUM_SIZE = 4; // This is the size of an individual field in a structure. 1 would
                  // lead to e.g. doubles and chars both taking 1 memory address. This
                  // is a form of 'compressed' memory, with shrinking and stretching
                  // according to the type, when compared to C/C++. On the other hand
                  // 8 means all fields take 8 memory addresses, so a double takes
                  // the same as a char. Note that we only actually store something in
                  // the top address - the others are just empty, an 'alignment cost'
                  // of sorts.
                  //
                  // 1 is somewhat faster, but dangerous.
                  //
                  // TODO: Cleverly analyze malloc, memset, memcpy etc. operations in
                  //       llvm, and replace with the proper values for us

CORRECT_SIGNS = 1; // Whether we make sure to convert unsigned values to signed values.
                   // Decreases performance with additional runtime checks. Might not be
                   // needed in some kinds of code.
                   // If equal to 2, done on a line-by-line basis according to
                   // CORRECT_SIGNS_LINES
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

// Code embetterments
OPTIMIZE = 0; // Optimize llvm operations into js commands
RELOOP = 0; // Recreate js native loops from llvm data
USE_TYPED_ARRAYS = 0; // Try to use typed arrays for the heap

// Generated code debugging options
SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
               // If equal to 2, done on a line-by-line basis according to
               // SAFE_HEAP_LINES (note that these are the lines to *exclude*
               // from checking - the opposite of what CORRECT_*_LINES mean)
SAFE_HEAP_LOG = 0; // Print out every single heap read and write (LOTS of output)
LABEL_DEBUG = 0; // Print out labels and functions as we enter them
EXCEPTION_DEBUG = 1; // Print out exceptions in emscriptened code
EXECUTION_TIMEOUT = -1; // Throw an exception after X seconds - useful to debug infinite loops
CHECK_OVERFLOWS = 0; // Add code that checks for overflows in integer math operations.
                     // There is currently not much to do to handle overflows if they occur.
                     // We can add code to simulate i32/i64 overflows in JS, but that would
                     // be very slow. It probably makes more sense to avoid overflows in
                     // C/C++ code. For example, if you have an int that you multiply by
                     // some factor, in order to get 'random' hash values - by taking
                     // that |value & hash_table_size| - then multiplying enough times will overflow.
                     // But instead, you can do |value = value & 30_BITS| in each iteration.
CORRECT_OVERFLOWS = 1; // Experimental code that tries to prevent unexpected JS overflows in integer
                       // mathops, by doing controlled overflows (sort of parallel to a CPU).
                       // Note that as mentioned above in CHECK_OVERFLOWS, the best thing is to
                       // not rely on overflows in your C/C++ code, as even if this option works,
                       // it slows things down.
                       //
                       // If equal to 2, done on a line-by-line basis according to
                       // CORRECT_OVERFLOWS_LINES
                       //
                       // NOTE: You can introduce signing issues by using this option. If you
                       //       take a large enough 32-bit value, and correct it for overflows,
                       //       you may get a negative number, as JS & operations are signed.
CORRECT_ROUNDINGS = 1; // C rounds to 0 (-5.5 to -5, +5.5 to 5), while JS has no direct way to do that:
                       // Math.floor is to negative, ceil to positive. With CORRECT_ROUNDINGS,
                       // we will do slow but correct C rounding operations.

EXPORTED_FUNCTIONS = ['_main']; // Functions that are explicitly exported, so they are guaranteed to
                                // be accessible outside of the generated code.

SHOW_LABELS = 0; // Show labels in the generated code

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

