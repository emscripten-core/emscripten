// Tuning
QUANTUM_SIZE = 1; // This is the size of an individual field in a structure. 1 would
                  // lead to e.g. doubles and chars both taking 1 memory address. This
                  // is a form of 'compressed' memory, with shrinking and stretching
                  // according to the type, when compared to C/C++. On the other hand
                  // 8 means all fields take 8 memory addresses, so a double takes
                  // the same as a char. Note that we only actually store something in
                  // the top address - the others are just empty, an 'alignment cost'
                  // of sorts.
                  //
                  // llvm-gcc works with 1. However, clang uses llvm_memcpy for various
                  // things, and the number of bytes it copies is hardcoded. A simple
                  // way to prevent problems with that is to set QUANTUM_SIZE to 8.
                  // See the 'copyop' automatic test.

GUARD_SIGNS = 1; // Whether we make sure to convert unsigned values to signed values.
                 // Decreases performance with additional runtime checks. Might not be
                 // needed in some kinds of code.
GUARD_LABELS = 0; // Whether we should throw if we encounter a bad __label__, i.e.,
                  // if code flow runs into a fault

// Code embetterments
OPTIMIZE = 0; // Optimize llvm operations into js commands
RELOOP = 0; // Recreate js native loops from llvm data

// Generated code debugging options
SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
LABEL_DEBUG = 0; // Print out labels and functions as we enter them
EXCEPTION_DEBUG = 1; // Print out exceptions in emscriptened code
EXECUTION_TIMEOUT = -1; // Throw an exception after X seconds - useful to debug infinite loops

// Compiler debugging options
DEBUG_TAGS_SHOWING = ['enzymatic'];
  // Some useful items:
  //    gconst
  //    types
  //    relooping

