// Code embetterments
OPTIMIZE = 1; // Optimize llvm operations into js commands
RELOOP = 0; // Recreate js native loops from llvm data XXX - disabled pending optimizing rewrite

// Generated code debugging options
SAFE_HEAP = 0; // Check each write to the heap against a list of blocked addresses
LABEL_DEBUG = 0; // Print out labels and functions as we enter them
EXECUTION_TIMEOUT = -1; // Throw an exception after X seconds - useful to debug infinite loops

// Compiler debugging options
DEBUG_TAGS_SHOWING = ['enzymatic'];
  // Some useful items:
  //    gconst
  //    types
  //    relooping

