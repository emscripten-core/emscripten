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
// NOTE: As of now, ES_SIZEOF is not needed when using QUANTUM_SIZE
//       of 4. We will use the same offsets as C/C++ does in that case.
//       ES_SIZEOF is useful if QUANTUM_SIZE is 1.
//
// A 'safe' sizeof operator. Sadly llvm-gcc calculates sizeof's
// and stores them hardcoded as raw values, unlike say offsets
// within a structure which it nicely details using getelementptr.
//
// You should always use ES_SIZEOF|V instead of sizeof when using
// Emscripten. Use ES_SIZEOF for types, ES_SIZEOV for values.
//
// Note that there is no way for Emscripten to know if you used
// ES_SIZEOF properly, or if you did and and you used sizeof.
// No warning will be shown if you do not use it.
//
// Sadly
//        #define ES_SIZEOF(x) int(&((x*)(NULL))[1])
// does not work, since the compiler parses and hard-codes the
// value. So we need to trick it with a function call.
#ifdef EMSCRIPTEN
  template<class T>
  int es_sizeof(T* x) { return int(&x[1]); }
  #define ES_SIZEOF(T) es_sizeof((T*)0)
  template<class T>
  int es_sizeov(T* x) { return es_sizeof((T*)0); }
  #define ES_SIZEOV(V) es_sizeof(V)

  // Undefine normal archs, so asm is not attempted
  #define __EMSCRIPTEN__
  #undef __i386__
  #undef __x86_64__
#else
  #define ES_SIZEOF(T) sizeof(T)
#endif

// Interface to the underlying JS engine. This function will
// eval() the given script.
extern void emscripten_run_script(const char *script);

