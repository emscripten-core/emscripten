/**
 * This file contains a few useful things for compiling C/C++ code
 * with Emscripten, an LLVM-to-JavaScript compiler.
 *
 * The code can be used permissively under the MIT license.
 *
 * http://emscripten.org
 */

// ES_SIZEOF
//
// A 'safe' sizeof operator. Sadly llvm-gcc calculates sizeof's
// and stores them hardcoded as raw values, unlike say offsets
// within a structure which it nicely details using getelementptr.
//
// You should always use ES_SIZEOF instead of sizeof when using
// Emscripten.
//
// Note that this only works with types, not values.
//
// Sadly
//        #define ES_SIZEOF(x) int(&((x*)(NULL))[1])
// does not work, since the compiler parses and hard-codes the
// value. So we need to trick it with a function call.
#ifdef EMSCRIPTEN
  template<class T>
  int es_sizeof(T* x) { return int(&x[1]); }
  #define ES_SIZEOF(T) es_sizeof((T*)NULL)
#else
  #define ES_SIZEOF(T) sizeof(T)
#endif

