//==============================================================================
// Optimizer tool. This is meant to be run after the emscripten compiler has
// finished generating code. These optimizations are done on the generated
// code to further improve it.
//
// Be aware that this is *not* a general JS optimizer. It assumes that the
// input is valid asm.js and makes strong assumptions based on this. It may do
// anything from crashing to optimizing incorrectly if the input is not valid!
//==============================================================================

#include "simple_ast.h"
#include "optimizer.h"

#include <string.h> // only use this for param checking

using namespace cashew;

int main(int argc, char **argv) {
  // Read directives
  for (int i = 2; i < argc; i++) {
    std::string str(argv[i]);
    if (str == "asm") {} // the only possibility for us
    else if (str == "asmPreciseF32") preciseF32 = true;
    else if (str == "receiveJSON") receiveJSON = true;
    else if (str == "emitJSON") emitJSON = true;
    else if (str == "minifyWhitespace") minifyWhitespace = true;
    else if (str == "last") last = true;
  }

#ifdef PROFILING
    std::string str("reading and parsing");
    clock_t start = clock();
    errv("starting %s", str.c_str());
#endif

  // Read input file
  FILE *f = fopen(argv[1], "r");
  assert(f);
  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  char *input = new char[size+1];
  rewind(f);
  int num = fread(input, 1, size, f);
  // On Windows, ftell() gives the byte position (\r\n counts as two bytes), but when
  // reading, fread() returns the number of characters read (\r\n is read as one char \n, and counted as one),
  // so return value of fread can be less than size reported by ftell, and that is normal.
  assert((num > 0 || size == 0) && num <= size);
  fclose(f);
  input[num] = 0;

  char *extraInfoStart = strstr(input, "// EXTRA_INFO:");
  if (extraInfoStart) {
    extraInfo = arena.alloc();
    extraInfo->parse(extraInfoStart + 14);
    *extraInfoStart = 0; // ignore extra info when parsing
  }

  Ref doc;

  if (receiveJSON) {
    // Parse JSON source into the document
    doc = arena.alloc();
    doc->parse(input);
  } else {
    cashew::Parser<Ref, ValueBuilder> builder;
    doc = builder.parseToplevel(input);
  }
  // do not free input, its contents are used as strings

#ifdef PROFILING
    errv("    %s took %lu milliseconds", str.c_str(), (clock() - start)/1000);
#endif

  // Run passes on the Document
  for (int i = 2; i < argc; i++) {
    std::string str(argv[i]);
#ifdef PROFILING
    clock_t start = clock();
    errv("starting %s", str.c_str());
#endif
    bool worked = true;
    if (str == "asm") { worked = false; } // the default for us
    else if (str == "asmPreciseF32") { worked = false; }
    else if (str == "receiveJSON" || str == "emitJSON") { worked = false; }
    else if (str == "eliminateDeadFuncs") eliminateDeadFuncs(doc);
    else if (str == "eliminate") eliminate(doc);
    else if (str == "eliminateMemSafe") eliminateMemSafe(doc);
    else if (str == "simplifyExpressions") simplifyExpressions(doc);
    else if (str == "optimizeFrounds") optimizeFrounds(doc);
    else if (str == "simplifyIfs") simplifyIfs(doc);
    else if (str == "registerize") registerize(doc);
    else if (str == "registerizeHarder") registerizeHarder(doc);
    else if (str == "minifyLocals") minifyLocals(doc);
    else if (str == "minifyWhitespace") { worked = false; }
    else if (str == "asmLastOpts") asmLastOpts(doc);
    else if (str == "last") { worked = false; }
    else if (str == "noop") { worked = false; }
    else {
      fprintf(stderr, "unrecognized argument: %s\n", str.c_str());
      abort();
    }
#ifdef PROFILING
    errv("    %s took %lu milliseconds", str.c_str(), (clock() - start)/1000);
#endif
#ifdef DEBUGGING
    if (worked) {
      std::cerr << "ast after " << str << ":\n";
      doc->stringify(std::cerr);
      std::cerr << "\n";
    }
#endif
  }

  // Emit
  if (emitJSON) {
    doc->stringify(std::cout);
    std::cout << "\n";
  } else {
    JSPrinter jser(!minifyWhitespace, last, doc);
    jser.printAst();
    std::cout << jser.buffer << "\n";
  }
  return 0;
}

