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

GUARD_SIGNS = 1; // Whether we make sure to convert unsigned values to signed values.
                 // Decreases performance with additional runtime checks. Might not be
                 // needed in some kinds of code.
GUARD_LABELS = 0; // Whether we should throw if we encounter a bad __label__, i.e.,
                  // if code flow runs into a fault
GUARD_MEMORY = 1; // Whether we should check that each allocation to the stack does not
                  // exceed it's size, whether all allocations (stack and static) are
                  // of positive size, etc.
INVOKE_RUN = 1; // Whether we will call run(). Disable if you embed the generated
                // code in your own, and will call run() yourself at the right time

// Code embetterments
OPTIMIZE = 0; // Optimize llvm operations into js commands
RELOOP = 0; // Recreate js native loops from llvm data
USE_TYPED_ARRAYS = 0; // Try to use typed arrays for the heap

// Generated code debugging options
SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
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

// Compiler debugging options
DEBUG_TAGS_SHOWING = [];
  // Some useful items:
  //    framework
  //    gconst
  //    types
  //    vars
  //    relooping
  //    unparsedFunctions

